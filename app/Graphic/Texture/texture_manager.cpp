#include "texture_manager.h"

#include <cstddef>
#include <iostream>

#include "WicTextureLoader12.h"
#include "d3dx12.h"
#include "graphic.h"

bool TextureManager::TextureUpload(const std::wstring& path,
  ComPtr<ID3D12Resource>& textureUpload,
  D3D12_PLACED_SUBRESOURCE_FOOTPRINT& footprint,
  ComPtr<ID3D12Resource>& texture_buffer_) {
  // load texture
  std::unique_ptr<uint8_t[]> decodedData;
  D3D12_SUBRESOURCE_DATA subresource;
  // ComPtr<ID3D12Resource> texture_buffer_;

  HRESULT hr = DirectX::LoadWICTextureFromFile(device_.Get(), path.c_str(), &texture_buffer_, decodedData, subresource);
  if (FAILED(hr)) return false;

  // init upload buffer and upload

  D3D12_RESOURCE_DESC textureDesc = texture_buffer_->GetDesc();

  // Configure footprint
  // D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
  UINT numRows = 0;
  UINT64 rowSizeInBytes = 0;
  UINT64 totalBytes = 0;

  device_->GetCopyableFootprints(&textureDesc,
    0,                // FirstSubresource
    1,                // NumSubresources
    0,                // BaseOffset
    &footprint,       // output layout structure
    &numRows,         // output row count (Height)
    &rowSizeInBytes,  // output actual data size per row (excluding alignment Padding)
    &totalBytes       // output total Buffer size
  );

  // Create upload buffer
  CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
  auto uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(totalBytes);

  // ComPtr<ID3D12Resource> textureUpload;
  hr = device_->CreateCommittedResource(
    &uploadHeapProps, D3D12_HEAP_FLAG_NONE, &uploadBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&textureUpload));

  if (FAILED(hr)) {
    std::cerr << "Failed to create texture upload buffer." << std::endl;
    return false;
  }
  upload_buffers_.push_back(textureUpload);

  // Copy CPU data to upload buffer
  UINT8* pMappedData = nullptr;
  hr = textureUpload->Map(0, nullptr, reinterpret_cast<void**>(&pMappedData));
  if (FAILED(hr)) {
    std::cerr << "Failed to map texture upload buffer." << std::endl;
    return false;
  }

  const UINT8* pSrcData = reinterpret_cast<const UINT8*>(subresource.pData);
  for (UINT y = 0; y < numRows; ++y) {
    memcpy(pMappedData + footprint.Offset + y * footprint.Footprint.RowPitch,
      pSrcData + y * subresource.RowPitch,
      static_cast<size_t>(rowSizeInBytes));
  }
  textureUpload->Unmap(0, nullptr);

  return true;
}

uint32_t TextureManager::CreateSrv(ComPtr<ID3D12Resource> texture_buffer) {
  auto allocation = heap_manager_->GetSrvAllocator().Allocate(1);
  D3D12_RESOURCE_DESC texDesc = texture_buffer->GetDesc();
  D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Format = texDesc.Format;                                             // DXGI_FORMAT_R8G8B8A8_UNORM; //RGBA(0.0f～1.0fに正規化)
  srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;  // 後述
  srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;                       // 2Dテクスチャ
  srvDesc.Texture2D.MipLevels = texDesc.MipLevels;                             // ミップマップは使用しないので1
  device_->CreateShaderResourceView(texture_buffer.Get(),                      // ビューと関連付けるバッファ
    &srvDesc,                                                                  // 先ほど設定したテクスチャ設定情報
    allocation.cpu);

  return allocation.index;
}

std::shared_ptr<Texture> TextureManager::LoadTexture(const std::wstring& path) {
  // check cache
  if (texture_cache_.count(path)) return texture_cache_[path];

  ComPtr<ID3D12Resource> textureUpload;
  D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
  ComPtr<ID3D12Resource> texture_buffer_;

  if (!TextureUpload(path, textureUpload, footprint, texture_buffer_)) return nullptr;

  graphic_->ExecuteSync([&](ID3D12GraphicsCommandList* cmd_list) {
    D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
    srcLocation.pResource = textureUpload.Get();
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    srcLocation.PlacedFootprint = footprint;

    D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
    dstLocation.pResource = texture_buffer_.Get();
    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLocation.SubresourceIndex = 0;

    cmd_list->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

    // barrier
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
      texture_buffer_.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    cmd_list->ResourceBarrier(1, &barrier);
  });

  auto texture = std::make_shared<Texture>();
  texture->resource = texture_buffer_;
  texture->srv_index = CreateSrv(texture_buffer_);  // important: this is the bindless index

  texture_cache_[path] = texture;
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

  struct UploadTask {
    ComPtr<ID3D12Resource> texture_buffer;
    ComPtr<ID3D12Resource> upload_buffer;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
    std::wstring path;
  };
  std::vector<UploadTask> tasks;

  for (const auto& path : paths_to_load) {
    ComPtr<ID3D12Resource> textureUpload;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
    ComPtr<ID3D12Resource> texture_buffer;

    TextureUpload(path, textureUpload, footprint, texture_buffer);

    tasks.push_back({texture_buffer, textureUpload, footprint, path});
  }

  graphic_->ExecuteSync([&](ID3D12GraphicsCommandList* cmd_list) {
    for (auto& task : tasks) {
      D3D12_TEXTURE_COPY_LOCATION src = {};
      src.pResource = task.upload_buffer.Get();
      src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
      src.PlacedFootprint = task.footprint;

      D3D12_TEXTURE_COPY_LOCATION dst = {};
      dst.pResource = task.texture_buffer.Get();
      dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
      dst.SubresourceIndex = 0;

      cmd_list->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

      auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        task.texture_buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
      cmd_list->ResourceBarrier(1, &barrier);
    }
  });

  for (auto& task : tasks) {
    auto tex = std::make_shared<Texture>();
    tex->resource = task.texture_buffer;
    tex->srv_index = CreateSrv(task.texture_buffer);

    texture_cache_[task.path] = tex;
    results.push_back(tex);
    upload_buffers_.push_back(task.upload_buffer);
  }

  return results;
}
