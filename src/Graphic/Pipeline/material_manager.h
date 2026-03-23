#pragma once
#include <d3d12.h>
#include <d3d12shader.h>
#include <dxgiformat.h>

#include <atomic>
#include <memory>
#include <shared_mutex>
#include <unordered_map>

#include "Framework/Render/render_settings.h"
#include "Pipeline/shader_manager.h"
#include "Pipeline/shader_types.h"
#include "material.h"

class ShaderManager;

class MaterialManager {
 public:
  MaterialManager() = default;
  ~MaterialManager() = default;

  bool Initialize(ID3D12Device* device, ShaderManager* shader_manager);

  // Get or create material based on shader ID and render settings
  Material* GetOrCreateMaterial(ShaderId shader_id, const Rendering::RenderSettings& settings);

  // Template overload for type-safe shader access
  template <typename ShaderType>
  Material* GetOrCreateMaterial(const Rendering::RenderSettings& settings) {
    return GetOrCreateMaterial(ShaderType::ID, settings);
  }

  // Get default render settings for a shader
  static Rendering::RenderSettings GetDefaultSettings(ShaderId shader_id);

  void SetWireframeOverride(bool enabled) {
    wireframe_override_ = enabled;
  }
  bool IsWireframeOverride() const {
    return wireframe_override_;
  }

  // Frame lifecycle
  void OnFrameEnd();

 private:
  ID3D12Device* device_ = nullptr;
  ShaderManager* shader_manager_ = nullptr;
  bool wireframe_override_ = false;

  // === Unified LRU Cache ===
  struct CacheEntry {
    std::unique_ptr<Material> material;
    std::atomic<uint64_t> last_used_frame{0};

    CacheEntry() = default;
    CacheEntry(CacheEntry&& other) noexcept
        : material(std::move(other.material)), last_used_frame(other.last_used_frame.load(std::memory_order_relaxed)) {
    }
    CacheEntry& operator=(CacheEntry&& other) noexcept {
      material = std::move(other.material);
      last_used_frame.store(other.last_used_frame.load(std::memory_order_relaxed), std::memory_order_relaxed);
      return *this;
    }
  };

  static constexpr size_t MAX_CACHE_SIZE = 128;
  std::unordered_map<uint64_t, CacheEntry> unified_cache_;
  std::shared_mutex cache_mutex_;
  uint64_t current_frame_ = 0;

  // === Helper Methods ===

  Material CreateMaterialInternal(ShaderId shader_id, const Rendering::RenderSettings& settings);

  void EvictLRU();

  uint64_t GenerateSortKey(ID3D12RootSignature* rs, ID3D12PipelineState* pso) const {
    uint32_t rs_hash = HashPointer(rs);
    uint32_t pso_hash = HashPointer(pso);
    return (static_cast<uint64_t>(rs_hash) << 32) | pso_hash;
  }

  uint64_t GenerateCacheKey(ShaderId shader_id, const Rendering::RenderSettings& settings) const {
    return (static_cast<uint64_t>(shader_id) << 32) | settings.GetCacheKey();
  }

  uint32_t HashPointer(void* ptr) const {
    uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
    uint32_t hash = static_cast<uint32_t>(addr ^ (addr >> 32));
    hash ^= (hash >> 16);
    return hash;
  }
};
