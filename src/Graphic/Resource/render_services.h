#pragma once

#include <d3d12.h>

#include <cstdint>
#include <functional>
#include <memory>

class DescriptorHeapManager;
class TextureManager;
class MaterialManager;
class ShaderManager;

namespace Font {
class SpriteFontManager;
}

namespace gfx {

class RenderServices {
 public:
  using ExecuteSyncFn = std::function<void(std::function<void(ID3D12GraphicsCommandList*)>)>;
  using GetFenceValueFn = std::function<uint64_t()>;

  struct CreateInfo {
    ID3D12Device* device = nullptr;
    DescriptorHeapManager* heap_manager = nullptr;
    ExecuteSyncFn execute_sync;
    GetFenceValueFn get_current_fence_value;
    uint32_t frame_buffer_count = 2;
  };

  [[nodiscard]] static std::unique_ptr<RenderServices> Create(const CreateInfo& info);

  RenderServices(const RenderServices&) = delete;
  RenderServices& operator=(const RenderServices&) = delete;
  ~RenderServices();

  TextureManager& GetTextureManager() {
    return *texture_manager_;
  }
  MaterialManager& GetMaterialManager() {
    return *material_manager_;
  }
  ShaderManager& GetShaderManager() {
    return *shader_manager_;
  }
  Font::SpriteFontManager& GetFontManager() {
    return *font_manager_;
  }

  void OnFrameBegin(uint32_t frame_index, uint64_t completed_fence);
  void OnFrameEnd();

 private:
  RenderServices() = default;
  bool Initialize(const CreateInfo& info);

  std::unique_ptr<TextureManager> texture_manager_;
  std::unique_ptr<MaterialManager> material_manager_;
  std::unique_ptr<ShaderManager> shader_manager_;
  std::unique_ptr<Font::SpriteFontManager> font_manager_;

  GetFenceValueFn get_current_fence_value_;
  uint32_t frame_buffer_count_ = 2;
};

}  // namespace gfx
