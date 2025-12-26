#include "texture_manager.h"

#include <algorithm>
#include <iostream>

#include "Core/utils.h"
#include "d3dx12.h"
#include "graphic.h"

bool TextureManager::LoadAndGenerateMipmaps(const std::wstring& path, DirectX::ScratchImage& mipChain) {
  using namespace DirectX;

  std::wstring ext = path.substr(path.find_last_of(L'.') + 1);
  std::ranges::transform(ext, ext.begin(), ::towlower);

  TexMetadata metadata;
  ScratchImage image;
  HRESULT hr = E_FAIL;

  // load texture
  if (ext == L"dds") {
    hr = LoadFromDDSFile(path.c_str(), DDS_FLAGS_NONE, &metadata, image);
  } else if (ext == L"tga") {
    hr = LoadFromTGAFile(path.c_str(), &metadata, image);
  } else if (ext == L"hdr") {
    hr = LoadFromHDRFile(path.c_str(), &metadata, image);
  } else {
    hr = LoadFromWICFile(path.c_str(), WIC_FLAGS_NONE, &metadata, image);
  }

  if (FAILED(hr)) {
    std::wcerr << L"Failed to load texture: " << path << std::endl;
    return false;
  }

  // if mipmap are already available
  if (metadata.mipLevels > 1) {
    mipChain = std::move(image);
    return true;
  }

  // generate mipmap
  hr = GenerateMipMaps(image.GetImages(),
    image.GetImageCount(),
    image.GetMetadata(),
    TEX_FILTER_DEFAULT,  // TEX_FILTER_DEFAULT, TEX_FILTER_POINT, TEX_FILTER_LINEAR, TEX_FILTER_CUBIC
    0,                   // 0 = generate complete mipmap chain
    mipChain);

  if (FAILED(hr)) {
    std::cerr << "Failed to generate mipmaps for: " << utils::wstring_to_utf8(path) << std::endl;
    return false;
  }

  return true;
}

bool TextureManager::CreateTextureResource(const DirectX::TexMetadata& metadata, ComPtr<ID3D12Resource>& texture) {
  // texture desc
  D3D12_RESOURCE_DESC texDesc = {};
  texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  texDesc.Width = static_cast<UINT>(metadata.width);
  texDesc.Height = static_cast<UINT>(metadata.height);
  texDesc.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize);
  texDesc.MipLevels = static_cast<UINT16>(metadata.mipLevels);
  texDesc.Format = metadata.format;
  texDesc.SampleDesc.Count = 1;
  texDesc.SampleDesc.Quality = 0;
  texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

  // create default heap texture reosurce
  CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);

  HRESULT hr = device_->CreateCommittedResource(
    &defaultHeapProps, D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&texture));

  if (FAILED(hr)) {
    std::cerr << "Failed to create texture resource." << std::endl;
    return false;
  }

  return true;
}

bool TextureManager::PrepareUpload(const DirectX::ScratchImage& mipChain, ComPtr<ID3D12Resource> texture, UploadInfo& uploadInfo) {
  const auto& metadata = mipChain.GetMetadata();

  // prepare subresources
  uploadInfo.subresources.resize(metadata.mipLevels);
  for (size_t i = 0; i < metadata.mipLevels; ++i) {
    const DirectX::Image* img = mipChain.GetImage(i, 0, 0);
    uploadInfo.subresources[i].pData = img->pixels;
    uploadInfo.subresources[i].RowPitch = static_cast<LONG_PTR>(img->rowPitch);
    uploadInfo.subresources[i].SlicePitch = static_cast<LONG_PTR>(img->slicePitch);
  }

  // calculate upload buffer size
  UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, static_cast<UINT>(metadata.mipLevels));

  // create upload buffer
  CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
  auto uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

  HRESULT hr = device_->CreateCommittedResource(&uploadHeapProps,
    D3D12_HEAP_FLAG_NONE,
    &uploadBufferDesc,
    D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(&uploadInfo.upload_buffer));

  if (FAILED(hr)) {
    std::cerr << "Failed to create upload buffer." << std::endl;
    return false;
  }

  return true;
}

uint32_t TextureManager::CreateSrv(ComPtr<ID3D12Resource> texture_buffer) {
  auto allocation = heap_manager_->GetSrvStaticAllocator().Allocate(1);
  D3D12_RESOURCE_DESC texDesc = texture_buffer->GetDesc();
  D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Format = texDesc.Format;                                             // DXGI_FORMAT_R8G8B8A8_UNORM; //RGBA(0.0f～1.0fに正規化)
  srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;  // 後述
  srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;                       // 2Dテクスチャ
  srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
  device_->CreateShaderResourceView(texture_buffer.Get(),  // ビューと関連付けるバッファ
    &srvDesc,                                              // 先ほど設定したテクスチャ設定情報
    allocation.cpu);

  return allocation.index;
}

std::shared_ptr<Texture> TextureManager::LoadTexture(const std::wstring& path) {
  // check cache
  if (texture_cache_.count(path)) return texture_cache_[path];

  ComPtr<ID3D12Resource> texture_buffer_;

  DirectX::ScratchImage mipChain;
  if (!LoadAndGenerateMipmaps(path, mipChain)) {
    return nullptr;
  }

  ComPtr<ID3D12Resource> texture_buffer;
  if (!CreateTextureResource(mipChain.GetMetadata(), texture_buffer)) {
    return nullptr;
  }

  UploadInfo uploadInfo;
  if (!PrepareUpload(mipChain, texture_buffer, uploadInfo)) {
    return nullptr;
  }

  graphic_->ExecuteSync([&](ID3D12GraphicsCommandList* command_list) {
    UpdateSubresources(command_list,
      texture_buffer.Get(),
      uploadInfo.upload_buffer.Get(),
      0,
      0,
      static_cast<UINT>(uploadInfo.subresources.size()),
      uploadInfo.subresources.data());

    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
      texture_buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    command_list->ResourceBarrier(1, &barrier);
  });

  {
    std::lock_guard<std::mutex> lock(upload_mutex_);
    upload_buffers_.push_back(uploadInfo.upload_buffer);
  }

  auto texture = std::make_shared<Texture>();
  texture->resource = texture_buffer;
  texture->srv_index = CreateSrv(texture_buffer);  // important: this is the bindless index
  texture_cache_[path] = texture;

  D3D12_RESOURCE_DESC texDesc = texture_buffer->GetDesc();
  std::cout << "[Texture] Loaded \"" << utils::wstring_to_utf8(path) << "\" | " << texDesc.Width << "x" << texDesc.Height << " | "
            << texDesc.MipLevels << " mips | " << "Format: " << utils::GetDxgiFormatName(texDesc.Format) << " | "
            << "SRV Index: " << texture->srv_index << std::endl;

  return texture;
}

std::vector<std::shared_ptr<Texture>> TextureManager::LoadTextures(const std::vector<std::wstring>& paths) {
  std::vector<std::shared_ptr<Texture>> results;
  std::vector<std::wstring> paths_to_load;

  for (const auto& path : paths) {
    if (texture_cache_.count(path)) {
      results.push_back(texture_cache_[path]);
    } else {
      paths_to_load.push_back(path);
    }
  }

  if (paths_to_load.empty()) return results;

  struct LoadTask {
    ComPtr<ID3D12Resource> texture_buffer;
    UploadInfo uploadInfo;
    std::wstring path;
    DirectX::ScratchImage mipChain;
  };

  std::vector<LoadTask> tasks;
  for (const auto& path : paths_to_load) {
    LoadTask task;
    task.path = path;

    if (!LoadAndGenerateMipmaps(path, task.mipChain)) continue;
    if (!CreateTextureResource(task.mipChain.GetMetadata(), task.texture_buffer)) continue;
    if (!PrepareUpload(task.mipChain, task.texture_buffer, task.uploadInfo)) continue;

    tasks.push_back(std::move(task));
  }

  graphic_->ExecuteSync([&](ID3D12GraphicsCommandList* command_list) {
    for (auto& task : tasks) {
      UpdateSubresources(command_list,
        task.texture_buffer.Get(),
        task.uploadInfo.upload_buffer.Get(),
        0,
        0,
        static_cast<UINT>(task.uploadInfo.subresources.size()),
        task.uploadInfo.subresources.data());

      auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        task.texture_buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
      command_list->ResourceBarrier(1, &barrier);
    }
  });

  for (auto& task : tasks) {
    auto texture = std::make_shared<Texture>();
    texture->resource = task.texture_buffer;
    texture->srv_index = CreateSrv(task.texture_buffer);
    texture_cache_[task.path] = texture;
    results.push_back(texture);

    D3D12_RESOURCE_DESC texDesc = task.texture_buffer->GetDesc();
    std::cout << "[Texture] Loaded \"" << utils::wstring_to_utf8(task.path) << "\" | " << texDesc.Width << "x" << texDesc.Height << " | "
              << texDesc.MipLevels << " mips | " << "Format: " << utils::GetDxgiFormatName(texDesc.Format) << " | "
              << "SRV Index: " << texture->srv_index << std::endl;

    std::lock_guard<std::mutex> lock(upload_mutex_);
    upload_buffers_.push_back(task.uploadInfo.upload_buffer);
  }

  return results;
}
