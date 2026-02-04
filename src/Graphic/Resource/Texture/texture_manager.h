#pragma once

#include <DirectXTex.h>
#include <d3d12.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <ranges>
#include <unordered_map>
#include <vector>

#include "Core/types.h"
#include "Core/utils.h"
#include "Descriptor/descriptor_heap_manager.h"
#include "texture.h"

// Texture usage type for proper color space handling
enum class TextureUsage {
  Color,  // Albedo, diffuse, UI - use sRGB
  Data    // Normal, roughness, metallic - use linear
};

class TextureManager {
 public:
  // Callback types to decouple from Graphic
  using ExecuteSyncFn = std::function<void(std::function<void(ID3D12GraphicsCommandList*)>)>;
  using GetFenceValueFn = std::function<uint64_t()>;

  bool Initialize(ID3D12Device* device,
    DescriptorHeapManager* heap_manager,
    ExecuteSyncFn execute_sync,
    GetFenceValueFn get_fence_value,
    uint32_t frame_buffer_count) {
    device_ = device;
    heap_manager_ = heap_manager;
    execute_sync_ = std::move(execute_sync);
    get_fence_value_ = std::move(get_fence_value);
    frame_buffer_count_ = frame_buffer_count;
    return true;
  }

  // load and return texture
  std::shared_ptr<Texture> LoadTexture(const std::wstring& path);
  std::shared_ptr<Texture> LoadTexture(const std::string& path) {
    return LoadTexture(utils::utf8_to_wstring(path));
  }

  // NEW: Explicit sRGB and linear loading methods
  std::shared_ptr<Texture> LoadTextureSRGB(const std::wstring& path);
  std::shared_ptr<Texture> LoadTextureSRGB(const std::string& path) {
    return LoadTextureSRGB(utils::utf8_to_wstring(path));
  }
  std::shared_ptr<Texture> LoadTextureLinear(const std::wstring& path);
  std::shared_ptr<Texture> LoadTextureLinear(const std::string& path) {
    return LoadTextureLinear(utils::utf8_to_wstring(path));
  }

  // NEW: With explicit usage tracking (for debugging)
  std::shared_ptr<Texture> LoadTexture(const std::wstring& path, TextureUsage usage);
  std::shared_ptr<Texture> LoadTexture(const std::string& path, TextureUsage usage) {
    return LoadTexture(utils::utf8_to_wstring(path), usage);
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

  void UnloadTexture(const std::wstring& path);
  void UnloadTexture(const std::shared_ptr<Texture>& texture) {
    if (!texture || texture->source_path.empty()) return;
    UnloadTexture(texture->source_path);
  }

  // Called every frame to free resources that GPU is done with
  void ProcessDeferredFrees(uint64_t completed_fence_value);

 private:
  ComPtr<ID3D12Device> device_ = nullptr;
  DescriptorHeapManager* heap_manager_ = nullptr;
  ExecuteSyncFn execute_sync_;
  GetFenceValueFn get_fence_value_;
  uint32_t frame_buffer_count_ = 2;

  // cache
  std::unordered_map<std::wstring, std::shared_ptr<Texture>> texture_cache_;

  // upload buffer
  std::mutex upload_mutex_;
  std::vector<ComPtr<ID3D12Resource>> upload_buffers_;

  struct UploadInfo {
    ComPtr<ID3D12Resource> upload_buffer;
    std::vector<D3D12_SUBRESOURCE_DATA> subresources;
  };

  // NEW: Added force_srgb parameter
  bool LoadAndGenerateMipmaps(const std::wstring& path, DirectX::ScratchImage& mipChain, bool force_srgb);

  // NEW: Added use_srgb parameter
  bool CreateTextureResource(const DirectX::TexMetadata& metadata, ComPtr<ID3D12Resource>& texture, bool use_srgb);

  // Helper: Check if format can be converted to sRGB
  static bool CanConvertToSRGB(DXGI_FORMAT format);

  bool PrepareUpload(const DirectX::ScratchImage& mipChain, ComPtr<ID3D12Resource> texture, UploadInfo& uploadInfo);

  uint32_t CreateSrv(ComPtr<ID3D12Resource> texture_buffer);

  struct PendingDelete {
    std::shared_ptr<Texture> texture;  // Keep texture alive
    uint32_t descriptor_index;         // Which descriptor to free
    uint64_t fence_value;              // When GPU will be done with it
  };

  std::mutex texture_mutex_;  // Protects cache and pending deletes
  std::vector<PendingDelete> pending_deletes_;
};
