#pragma once
#include <d3d12.h>
#include <d3d12shader.h>
#include <dxgiformat.h>

#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>

#include "Core/types.h"
#include "Game/Component/render_settings.h"
#include "Pipeline/pipeline_state_builder.h"
#include "Pipeline/shader_manager.h"
#include "Pipeline/shader_types.h"
#include "material.h"

// Forward declaration
class ShaderManager;

// Centralized material management system
// Handles creation, storage, and retrieval of materials (PSO + Root Signature combinations)
class MaterialManager {
 public:
  MaterialManager() = default;
  ~MaterialManager() = default;

  // Initialize with device and shader manager
  bool Initialize(ID3D12Device* device, ShaderManager* shader_manager);

  // Get or create material based on shader ID and render settings
  Material* GetOrCreateMaterial(Graphics::ShaderID shader_id, const Rendering::RenderSettings& settings);

  // Get default render settings for a shader (static helper)
  static Rendering::RenderSettings GetDefaultSettings(Graphics::ShaderID shader_id);

  // Frame lifecycle
  void OnFrameEnd();

 private:
  ID3D12Device* device_ = nullptr;
  ShaderManager* shader_manager_ = nullptr;

  // === Unified LRU Cache ===
  // Cache key: hash(shader_id) ^ settings.GetCacheKey()
  struct CacheEntry {
    std::unique_ptr<Material> material;
    uint64_t last_used_frame;
  };

  static constexpr size_t MAX_CACHE_SIZE = 128;  // Increased for more shader combinations
  std::unordered_map<uint64_t, CacheEntry> unified_cache_;
  std::shared_mutex cache_mutex_;
  uint64_t current_frame_ = 0;

  // === Helper Methods ===

  // Unified material creation (internal)
  Material CreateMaterialInternal(Graphics::ShaderID shader_id, const Rendering::RenderSettings& settings);

  // LRU eviction
  void EvictLRU();

  // Sort Key Generation
  uint64_t GenerateSortKey(ID3D12RootSignature* rs, ID3D12PipelineState* pso) const {
    uint32_t rs_hash = HashPointer(rs);
    uint32_t pso_hash = HashPointer(pso);
    return (static_cast<uint64_t>(rs_hash) << 32) | pso_hash;
  }

  // Cache key generation for unified cache
  uint64_t GenerateCacheKey(Graphics::ShaderID shader_id, const Rendering::RenderSettings& settings) const {
    return (static_cast<uint64_t>(shader_id) << 32) | settings.GetCacheKey();
  }

  // Simple hash function for D3D12 object pointers
  uint32_t HashPointer(void* ptr) const {
    uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
    uint32_t hash = static_cast<uint32_t>(addr ^ (addr >> 32));
    hash ^= (hash >> 16);
    return hash;
  }
};
