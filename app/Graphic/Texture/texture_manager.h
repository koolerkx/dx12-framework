#pragma once

#include <d3d12.h>

#include <cstdint>
#include <memory>
#include <ranges>
#include <unordered_map>
#include <vector>

#include "descriptor_heap_manager.h"
#include "Core/types.h"
#include "Core/utils.h"

class Graphic;

struct Texture {
  ComPtr<ID3D12Resource> resource;
  uint32_t srv_index;

  uint32_t GetBindlessIndex() const {
    return srv_index;
  }
};

class TextureManager {
 public:
  bool Initialize(Graphic* graphic, ID3D12Device* device, DescriptorHeapManager* heap_manager) {
    graphic_ = graphic;
    device_ = device;
    heap_manager_ = heap_manager;
    return true;
  }

  // load and return texture
  std::shared_ptr<Texture> LoadTexture(const std::wstring& path);
  std::shared_ptr<Texture> LoadTexture(const std::string& path) {
    return LoadTexture(utils::utf8_to_wstring(path));
  }
  std::vector<std::shared_ptr<Texture>> LoadTextures(const std::vector<std::wstring>& paths);
  std::vector<std::shared_ptr<Texture>> LoadTextures(const std::vector<std::string>& paths) {
    return LoadTextures(paths | std::views::transform(utils::utf8_to_wstring) | std::ranges::to<std::vector>());
  }

  // Optional cleanup on each frame end
  void CleanUploadBuffers() {
    std::lock_guard<std::mutex> lock(upload_mutex_);
    upload_buffers_.clear();
  }

 private:
  Graphic* graphic_ = nullptr;
  ComPtr<ID3D12Device> device_ = nullptr;
  DescriptorHeapManager* heap_manager_ = nullptr;

  // cache
  std::unordered_map<std::wstring, std::shared_ptr<Texture>> texture_cache_;

  // upload buffer
  std::mutex upload_mutex_;
  std::vector<ComPtr<ID3D12Resource>> upload_buffers_;

  bool TextureUpload(const std::wstring& path,
    ComPtr<ID3D12Resource>& textureUpload,
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT& footprint,
    ComPtr<ID3D12Resource>& texture_buffer_);
  uint32_t CreateSrv(ComPtr<ID3D12Resource> texture_buffer);
};
