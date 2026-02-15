#include "texture_manager.h"

#include <algorithm>

#include "Core/utils.h"
#include "Framework/Logging/logger.h"
#include "d3dx12.h"

bool TextureManager::CanConvertToSRGB(DXGI_FORMAT format) {
  return format == DXGI_FORMAT_R8G8B8A8_UNORM || format == DXGI_FORMAT_B8G8R8A8_UNORM || format == DXGI_FORMAT_R8G8B8A8_TYPELESS ||
         format == DXGI_FORMAT_B8G8R8A8_TYPELESS;
}

bool TextureManager::LoadAndGenerateMipmaps(const std::wstring& path, DirectX::ScratchImage& mipChain, bool force_srgb) {
  using namespace DirectX;

  std::wstring ext = path.substr(path.find_last_of(L'.') + 1);
  std::ranges::transform(ext, ext.begin(), ::towlower);

  TexMetadata metadata;
  ScratchImage image;
  HRESULT hr = E_FAIL;

  // load texture based on extension
  if (ext == L"dds") {
    hr = LoadFromDDSFile(path.c_str(), DDS_FLAGS_NONE, &metadata, image);
    if (FAILED(hr)) {
      Logger::LogFormat(
        LogLevel::Error, LogCategory::Resource, Logger::Here(), "[Texture] Failed to load DDS: {}", utils::wstring_to_utf8(path));
      return false;
    }
    // For DDS, convert to sRGB if requested and format is compatible
    if (force_srgb && CanConvertToSRGB(metadata.format)) {
      hr = Convert(image.GetImages(), image.GetImageCount(), metadata, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_FILTER_DEFAULT, 0.5f, mipChain);
      if (FAILED(hr)) {
        Logger::LogFormat(LogLevel::Error,
          LogCategory::Resource,
          Logger::Here(),
          "[Texture] Failed to convert DDS to sRGB: {}",
          utils::wstring_to_utf8(path));
        return false;
      }
    } else if (metadata.mipLevels > 1) {
      mipChain = std::move(image);
    } else {
      hr = GenerateMipMaps(image.GetImages(), image.GetImageCount(), metadata, TEX_FILTER_DEFAULT, 0, mipChain);
    }
  } else if (ext == L"tga") {
    hr = LoadFromTGAFile(path.c_str(), &metadata, image);
    if (FAILED(hr)) {
      Logger::LogFormat(
        LogLevel::Error, LogCategory::Resource, Logger::Here(), "[Texture] Failed to load TGA: {}", utils::wstring_to_utf8(path));
      return false;
    }
    // Convert to sRGB if requested and format is compatible
    if (force_srgb && CanConvertToSRGB(metadata.format)) {
      hr = Convert(image.GetImages(), image.GetImageCount(), metadata, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, TEX_FILTER_DEFAULT, 0.5f, mipChain);
    } else {
      hr = GenerateMipMaps(image.GetImages(), image.GetImageCount(), metadata, TEX_FILTER_DEFAULT, 0, mipChain);
    }
  } else if (ext == L"hdr") {
    // HDR is always linear (floating point), ignore force_srgb
    hr = LoadFromHDRFile(path.c_str(), &metadata, image);
    if (FAILED(hr)) {
      Logger::LogFormat(
        LogLevel::Error, LogCategory::Resource, Logger::Here(), "[Texture] Failed to load HDR: {}", utils::wstring_to_utf8(path));
      return false;
    }
    hr = GenerateMipMaps(image.GetImages(), image.GetImageCount(), metadata, TEX_FILTER_DEFAULT, 0, mipChain);
  } else {
    // WIC formats: PNG, JPG, JPEG, BMP, etc.
    DirectX::WIC_FLAGS loadFlags = force_srgb ? DirectX::WIC_FLAGS_FORCE_SRGB : DirectX::WIC_FLAGS_NONE;
    hr = LoadFromWICFile(path.c_str(), loadFlags, &metadata, image);
    if (FAILED(hr)) {
      Logger::LogFormat(
        LogLevel::Error, LogCategory::Resource, Logger::Here(), "[Texture] Failed to load WIC texture: {}", utils::wstring_to_utf8(path));
      return false;
    }
    hr = GenerateMipMaps(image.GetImages(), image.GetImageCount(), metadata, TEX_FILTER_DEFAULT, 0, mipChain);
  }

  if (FAILED(hr)) {
    Logger::LogFormat(
      LogLevel::Error, LogCategory::Resource, Logger::Here(), "[Texture] Failed to generate mipmaps for: {}", utils::wstring_to_utf8(path));
    return false;
  }

  return true;
}

bool TextureManager::LoadFromMemoryAndGenerateMipmaps(const uint8_t* data, size_t size, DirectX::ScratchImage& mipChain, bool force_srgb) {
  using namespace DirectX;

  TexMetadata metadata;
  ScratchImage image;

  WIC_FLAGS load_flags = force_srgb ? WIC_FLAGS_FORCE_SRGB : WIC_FLAGS_NONE;
  HRESULT hr = LoadFromWICMemory(data, size, load_flags, &metadata, image);
  if (FAILED(hr)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Resource, Logger::Here(), "[Texture] Failed to load texture from memory");
    return false;
  }

  hr = GenerateMipMaps(image.GetImages(), image.GetImageCount(), metadata, TEX_FILTER_DEFAULT, 0, mipChain);
  if (FAILED(hr)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Resource, Logger::Here(), "[Texture] Failed to generate mipmaps for memory texture");
    return false;
  }

  return true;
}

bool TextureManager::LoadFromRawPixelsAndGenerateMipmaps(
  const uint8_t* pixels, uint32_t width, uint32_t height, DirectX::ScratchImage& mipChain, bool force_srgb) {
  using namespace DirectX;

  DXGI_FORMAT format = force_srgb ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;

  ScratchImage image;
  HRESULT hr = image.Initialize2D(format, width, height, 1, 1);
  if (FAILED(hr)) {
    Logger::LogFormat(
      LogLevel::Error, LogCategory::Resource, Logger::Here(), "[Texture] Failed to initialize raw pixel image {}x{}", width, height);
    return false;
  }

  const Image* dest = image.GetImage(0, 0, 0);
  const size_t src_row_bytes = static_cast<size_t>(width) * 4;
  for (uint32_t row = 0; row < height; ++row) {
    std::memcpy(dest->pixels + row * dest->rowPitch, pixels + row * src_row_bytes, src_row_bytes);
  }

  hr = GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), TEX_FILTER_DEFAULT, 0, mipChain);
  if (FAILED(hr)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Resource, Logger::Here(), "[Texture] Failed to generate mipmaps for raw pixel texture");
    return false;
  }

  return true;
}

std::shared_ptr<Texture> TextureManager::LoadTextureFromRawPixels(
  const std::string& cache_key, const uint8_t* pixels, uint32_t width, uint32_t height, bool force_srgb) {
  std::wstring wkey = utils::utf8_to_wstring(cache_key);
  if (texture_cache_.count(wkey)) return texture_cache_[wkey];

  DirectX::ScratchImage mipChain;
  if (!LoadFromRawPixelsAndGenerateMipmaps(pixels, width, height, mipChain, force_srgb)) {
    return nullptr;
  }

  ComPtr<ID3D12Resource> texture_buffer;
  if (!CreateTextureResource(mipChain.GetMetadata(), texture_buffer, force_srgb)) {
    return nullptr;
  }

  UploadInfo uploadInfo;
  if (!PrepareUpload(mipChain, texture_buffer, uploadInfo)) {
    return nullptr;
  }

  execute_sync_([&](ID3D12GraphicsCommandList* command_list) {
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
  texture->srv_index = CreateSrv(texture_buffer);
  texture->source_path = wkey;
  texture_cache_[wkey] = texture;

  D3D12_RESOURCE_DESC texDesc = texture_buffer->GetDesc();
  Logger::LogFormat(LogLevel::Info,
    LogCategory::Resource,
    Logger::Here(),
    "[Texture] Loaded raw pixels \"{}\" | {}x{} | {} mips | Format: {} | SRV Index: {}",
    cache_key,
    texDesc.Width,
    texDesc.Height,
    texDesc.MipLevels,
    utils::GetDxgiFormatName(texDesc.Format),
    texture->srv_index);

  return texture;
}

bool TextureManager::CreateTextureResource(const DirectX::TexMetadata& metadata, ComPtr<ID3D12Resource>& texture, bool use_srgb) {
  // texture desc
  D3D12_RESOURCE_DESC texDesc = {};
  texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  texDesc.Width = static_cast<UINT>(metadata.width);
  texDesc.Height = static_cast<UINT>(metadata.height);
  texDesc.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize);
  texDesc.MipLevels = static_cast<UINT16>(metadata.mipLevels);
  texDesc.Format = metadata.format;

  // Convert to sRGB format if requested and format is compatible
  if (use_srgb) {
    if (metadata.format == DXGI_FORMAT_R8G8B8A8_UNORM) {
      texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    } else if (metadata.format == DXGI_FORMAT_B8G8R8A8_UNORM) {
      texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    }
    // Note: DDS/TGA already converted in LoadAndGenerateMipmaps if needed
  }

  texDesc.SampleDesc.Count = 1;
  texDesc.SampleDesc.Quality = 0;
  texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

  // create default heap texture reosurce
  CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);

  HRESULT hr = device_->CreateCommittedResource(
    &defaultHeapProps, D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&texture));

  if (FAILED(hr)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Resource, Logger::Here(), "Failed to create texture resource");
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
    Logger::LogFormat(LogLevel::Error, LogCategory::Resource, Logger::Here(), "Failed to create upload buffer");
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
  // Default to sRGB for color textures (backward compatible)
  return LoadTextureSRGB(path);
}

// NEW: With explicit usage tracking
std::shared_ptr<Texture> TextureManager::LoadTexture(const std::wstring& path, TextureUsage usage) {
  return (usage == TextureUsage::Color) ? LoadTextureSRGB(path) : LoadTextureLinear(path);
}

// NEW: Load texture with sRGB color space
std::shared_ptr<Texture> TextureManager::LoadTextureSRGB(const std::wstring& path) {
  // check cache
  if (texture_cache_.count(path)) return texture_cache_[path];

  DirectX::ScratchImage mipChain;
  if (!LoadAndGenerateMipmaps(path, mipChain, true)) {
    return nullptr;
  }

  ComPtr<ID3D12Resource> texture_buffer;
  if (!CreateTextureResource(mipChain.GetMetadata(), texture_buffer, true)) {
    return nullptr;
  }

  UploadInfo uploadInfo;
  if (!PrepareUpload(mipChain, texture_buffer, uploadInfo)) {
    return nullptr;
  }

  execute_sync_([&](ID3D12GraphicsCommandList* command_list) {
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
  texture->srv_index = CreateSrv(texture_buffer);
  texture->source_path = path;
  texture_cache_[path] = texture;

  D3D12_RESOURCE_DESC texDesc = texture_buffer->GetDesc();
  Logger::LogFormat(LogLevel::Info,
    LogCategory::Resource,
    Logger::Here(),
    "[Texture] Loaded SRGB \"{}\" | {}x{} | {} mips | Format: {} | SRV Index: {}",
    utils::wstring_to_utf8(path),
    texDesc.Width,
    texDesc.Height,
    texDesc.MipLevels,
    utils::GetDxgiFormatName(texDesc.Format),
    texture->srv_index);

  return texture;
}

// NEW: Load texture with linear color space
std::shared_ptr<Texture> TextureManager::LoadTextureLinear(const std::wstring& path) {
  // Separate cache key for linear version
  std::wstring linear_key = path + L"_linear";
  if (texture_cache_.count(linear_key)) return texture_cache_[linear_key];

  DirectX::ScratchImage mipChain;
  if (!LoadAndGenerateMipmaps(path, mipChain, false)) {
    return nullptr;
  }

  ComPtr<ID3D12Resource> texture_buffer;
  if (!CreateTextureResource(mipChain.GetMetadata(), texture_buffer, false)) {
    return nullptr;
  }

  UploadInfo uploadInfo;
  if (!PrepareUpload(mipChain, texture_buffer, uploadInfo)) {
    return nullptr;
  }

  execute_sync_([&](ID3D12GraphicsCommandList* command_list) {
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
  texture->srv_index = CreateSrv(texture_buffer);
  texture->source_path = linear_key;
  texture_cache_[linear_key] = texture;

  D3D12_RESOURCE_DESC texDesc = texture_buffer->GetDesc();
  Logger::LogFormat(LogLevel::Info,
    LogCategory::Resource,
    Logger::Here(),
    "[Texture] Loaded Linear \"{}\" | {}x{} | {} mips | Format: {} | SRV Index: {}",
    utils::wstring_to_utf8(path),
    texDesc.Width,
    texDesc.Height,
    texDesc.MipLevels,
    utils::GetDxgiFormatName(texDesc.Format),
    texture->srv_index);

  return texture;
}

std::shared_ptr<Texture> TextureManager::LoadTextureFromMemory(
  const std::string& cache_key, const uint8_t* data, size_t size, bool force_srgb) {
  std::wstring wkey = utils::utf8_to_wstring(cache_key);
  if (texture_cache_.count(wkey)) return texture_cache_[wkey];

  DirectX::ScratchImage mipChain;
  if (!LoadFromMemoryAndGenerateMipmaps(data, size, mipChain, force_srgb)) {
    return nullptr;
  }

  ComPtr<ID3D12Resource> texture_buffer;
  if (!CreateTextureResource(mipChain.GetMetadata(), texture_buffer, force_srgb)) {
    return nullptr;
  }

  UploadInfo uploadInfo;
  if (!PrepareUpload(mipChain, texture_buffer, uploadInfo)) {
    return nullptr;
  }

  execute_sync_([&](ID3D12GraphicsCommandList* command_list) {
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
  texture->srv_index = CreateSrv(texture_buffer);
  texture->source_path = wkey;
  texture_cache_[wkey] = texture;

  D3D12_RESOURCE_DESC texDesc = texture_buffer->GetDesc();
  Logger::LogFormat(LogLevel::Info,
    LogCategory::Resource,
    Logger::Here(),
    "[Texture] Loaded from memory \"{}\" | {}x{} | {} mips | Format: {} | SRV Index: {}",
    cache_key,
    texDesc.Width,
    texDesc.Height,
    texDesc.MipLevels,
    utils::GetDxgiFormatName(texDesc.Format),
    texture->srv_index);

  return texture;
}

uint32_t TextureManager::CreateCubemapSrv(ComPtr<ID3D12Resource> texture_buffer, DXGI_FORMAT format, uint32_t mip_levels) {
  auto allocation = heap_manager_->GetSrvStaticAllocator().Allocate(1);

  D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
  srv_desc.Format = format;
  srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
  srv_desc.TextureCube.MostDetailedMip = 0;
  srv_desc.TextureCube.MipLevels = mip_levels;
  srv_desc.TextureCube.ResourceMinLODClamp = 0.0f;
  device_->CreateShaderResourceView(texture_buffer.Get(), &srv_desc, allocation.cpu);

  return allocation.index;
}

std::shared_ptr<Texture> TextureManager::LoadCubemapFromCrossHDR(const std::wstring& path) {
  std::wstring cache_key = path + L"_cubemap";
  if (texture_cache_.count(cache_key)) return texture_cache_[cache_key];

  using namespace DirectX;

  TexMetadata metadata;
  ScratchImage image;
  HRESULT hr = LoadFromHDRFile(path.c_str(), &metadata, image);
  if (FAILED(hr)) {
    Logger::LogFormat(
      LogLevel::Error, LogCategory::Resource, Logger::Here(), "[Texture] Failed to load HDR cubemap: {}", utils::wstring_to_utf8(path));
    return nullptr;
  }

  // HDR may load as R32G32B32_FLOAT (96bpp) which DX12 doesn't support for Texture2D
  const ScratchImage* source = &image;

  uint32_t face_size = static_cast<uint32_t>(metadata.width) / 4;
  if (metadata.height != face_size * 3) {
    Logger::LogFormat(LogLevel::Error,
      LogCategory::Resource,
      Logger::Here(),
      "[Texture] HDR cross layout mismatch: {}x{}, expected height={}",
      metadata.width,
      metadata.height,
      face_size * 3);
    return nullptr;
  }

  // Cross layout face positions (col, row) in face_size units
  // +X=right(2,1) -X=left(0,1) +Y=top(1,0) -Y=bottom(1,2) +Z=front(1,1) -Z=back(3,1)
  struct FaceOffset {
    uint32_t col;
    uint32_t row;
  };
  constexpr FaceOffset face_offsets[6] = {{2, 1}, {0, 1}, {1, 0}, {1, 2}, {1, 1}, {3, 1}};

  size_t bytes_per_pixel = BitsPerPixel(metadata.format) / 8;
  size_t face_row_pitch = face_size * bytes_per_pixel;
  size_t src_row_pitch = source->GetImage(0, 0, 0)->rowPitch;

  // Extract 6 faces
  std::vector<std::vector<uint8_t>> face_data(6);
  std::vector<D3D12_SUBRESOURCE_DATA> subresources(6);

  for (int i = 0; i < 6; ++i) {
    face_data[i].resize(face_size * face_row_pitch);
    const uint8_t* src_pixels = source->GetImage(0, 0, 0)->pixels;

    uint32_t start_x = face_offsets[i].col * face_size;
    uint32_t start_y = face_offsets[i].row * face_size;

    for (uint32_t row = 0; row < face_size; ++row) {
      const uint8_t* src_row = src_pixels + (start_y + row) * src_row_pitch + start_x * bytes_per_pixel;
      uint8_t* dst_row = face_data[i].data() + row * face_row_pitch;
      memcpy(dst_row, src_row, face_row_pitch);
    }

    subresources[i].pData = face_data[i].data();
    subresources[i].RowPitch = static_cast<LONG_PTR>(face_row_pitch);
    subresources[i].SlicePitch = static_cast<LONG_PTR>(face_size * face_row_pitch);
  }

  // Create cubemap resource
  D3D12_RESOURCE_DESC tex_desc = {};
  tex_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  tex_desc.Width = face_size;
  tex_desc.Height = face_size;
  tex_desc.DepthOrArraySize = 6;
  tex_desc.MipLevels = 1;
  tex_desc.Format = metadata.format;
  tex_desc.SampleDesc.Count = 1;
  tex_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  tex_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

  CD3DX12_HEAP_PROPERTIES default_heap(D3D12_HEAP_TYPE_DEFAULT);
  ComPtr<ID3D12Resource> texture_buffer;
  hr = device_->CreateCommittedResource(
    &default_heap, D3D12_HEAP_FLAG_NONE, &tex_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&texture_buffer));
  if (FAILED(hr)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Resource, Logger::Here(), "[Texture] Failed to create cubemap resource");
    return nullptr;
  }

  // Upload
  UINT64 upload_size = GetRequiredIntermediateSize(texture_buffer.Get(), 0, 6);
  CD3DX12_HEAP_PROPERTIES upload_heap(D3D12_HEAP_TYPE_UPLOAD);
  auto upload_desc = CD3DX12_RESOURCE_DESC::Buffer(upload_size);

  ComPtr<ID3D12Resource> upload_buffer;
  hr = device_->CreateCommittedResource(
    &upload_heap, D3D12_HEAP_FLAG_NONE, &upload_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload_buffer));
  if (FAILED(hr)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Resource, Logger::Here(), "[Texture] Failed to create cubemap upload buffer");
    return nullptr;
  }

  execute_sync_([&](ID3D12GraphicsCommandList* cmd) {
    UpdateSubresources(cmd, texture_buffer.Get(), upload_buffer.Get(), 0, 0, 6, subresources.data());

    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
      texture_buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    cmd->ResourceBarrier(1, &barrier);
  });

  {
    std::lock_guard<std::mutex> lock(upload_mutex_);
    upload_buffers_.push_back(upload_buffer);
  }

  auto texture = std::make_shared<Texture>();
  texture->resource = texture_buffer;
  texture->srv_index = CreateCubemapSrv(texture_buffer, metadata.format, 1);
  texture->source_path = cache_key;
  texture_cache_[cache_key] = texture;

  Logger::LogFormat(LogLevel::Info,
    LogCategory::Resource,
    Logger::Here(),
    "[Texture] Loaded Cubemap \"{}\" | {}x{} per face | Format: {} | SRV Index: {}",
    utils::wstring_to_utf8(path),
    face_size,
    face_size,
    utils::GetDxgiFormatName(metadata.format),
    texture->srv_index);

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

    // Use sRGB by default to match LoadTexture() behavior
    if (!LoadAndGenerateMipmaps(path, task.mipChain, true)) continue;
    if (!CreateTextureResource(task.mipChain.GetMetadata(), task.texture_buffer, true)) continue;
    if (!PrepareUpload(task.mipChain, task.texture_buffer, task.uploadInfo)) continue;

    tasks.push_back(std::move(task));
  }

  execute_sync_([&](ID3D12GraphicsCommandList* command_list) {
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
    texture->source_path = task.path;
    texture_cache_[task.path] = texture;
    results.push_back(texture);

    D3D12_RESOURCE_DESC texDesc = task.texture_buffer->GetDesc();
    Logger::LogFormat(LogLevel::Info,
      LogCategory::Resource,
      Logger::Here(),
      "[Texture] Loaded \"{}\" | {}x{} | {} mips | Format: {} | SRV Index: {}",
      utils::wstring_to_utf8(task.path),
      texDesc.Width,
      texDesc.Height,
      texDesc.MipLevels,
      utils::GetDxgiFormatName(texDesc.Format),
      texture->srv_index);

    std::lock_guard<std::mutex> lock(upload_mutex_);
    upload_buffers_.push_back(task.uploadInfo.upload_buffer);
  }

  return results;
}

void TextureManager::UnloadTexture(const std::wstring& path) {
  std::lock_guard<std::mutex> lock(texture_mutex_);

  auto it = texture_cache_.find(path);
  if (it == texture_cache_.end()) {
    return;
  }

  // Safety check: Only unload if this is the last reference
  // ref_count > 1: other GameObjects/ Handler are still using this texture
  if (long ref_count = it->second.use_count(); ref_count > 1) {
    Logger::LogFormat(LogLevel::Warn,
      LogCategory::Resource,
      Logger::Here(),
      "[TextureManager] WARNING: Cannot unload \"{}\" - still has {} external references (must be exactly 1 to unload)",
      utils::wstring_to_utf8(path),
      ref_count - 1);
    return;
  }

  // Get safe fence value: current + buffer frames
  // This ensures all frames that might be using this texture will complete
  uint64_t current_fence = get_fence_value_();
  uint64_t safe_fence = current_fence + frame_buffer_count_;

  pending_deletes_.push_back({it->second, it->second->srv_index, safe_fence});

  texture_cache_.erase(it);
}

void TextureManager::ProcessDeferredFrees(uint64_t completed_fence_value) {
  std::lock_guard<std::mutex> lock(texture_mutex_);

  auto& allocator = heap_manager_->GetSrvStaticAllocator();

  auto it = pending_deletes_.begin();
  while (it != pending_deletes_.end()) {
    if (it->fence_value <= completed_fence_value) {
      allocator.FreeImmediate(it->descriptor_index, 1);

      it = pending_deletes_.erase(it);
    } else {
      ++it;
    }
  }

  // Periodic defragmentation
  static uint32_t free_counter = 0;
  if (++free_counter >= 16) {
    allocator.CoalesceFreeBlocks();
    free_counter = 0;
  }
}
