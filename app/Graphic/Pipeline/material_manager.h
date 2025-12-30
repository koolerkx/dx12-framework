#pragma once
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgiformat.h>

#include <memory>
#include <string>
#include <unordered_map>

#include "Core/types.h"
#include "Pipeline/pipeline_state_builder.h"
#include "material.h"
#include "shader_config.h"

// Render state configuration for PSO creation
struct RenderStateConfig {
  BlendMode blend_mode = BlendMode::Opaque;
  D3D12_CULL_MODE cull_mode = D3D12_CULL_MODE_BACK;
  bool depth_test_enabled = true;
  bool depth_write_enabled = true;
  D3D12_COMPARISON_FUNC depth_func = D3D12_COMPARISON_FUNC_LESS;
  DXGI_FORMAT rtv_format = DXGI_FORMAT_R8G8B8A8_UNORM;
  DXGI_FORMAT dsv_format = DXGI_FORMAT_D32_FLOAT;

  static RenderStateConfig Default() {
    return RenderStateConfig{};
  }

  static RenderStateConfig Transparent() {
    RenderStateConfig config;
    config.blend_mode = BlendMode::AlphaBlend;
    config.depth_write_enabled = false;  // Transparent objects don't write depth
    return config;
  }

  static RenderStateConfig UI() {
    RenderStateConfig config;
    config.blend_mode = BlendMode::AlphaBlend;
    config.depth_test_enabled = false;
    config.depth_write_enabled = false;
    config.cull_mode = D3D12_CULL_MODE_NONE;
    return config;
  }
};

// Centralized material management system
// Handles creation, storage, and retrieval of materials (PSO + Root Signature combinations)
class MaterialManager {
 public:
  MaterialManager() = default;
  ~MaterialManager() = default;

  // Initialize with device
  bool Initialize(ID3D12Device* device);

  // Material Creation API
  // Create material with default/shared root signature
  // This is the same API as before for backward compatibility
  Material* CreateMaterial(
    const std::string& name, const ShaderConfig& shader_config, const RenderStateConfig& render_state = RenderStateConfig::Default());

  // Create material with custom root signature (for special materials)
  Material* CreateMaterial(const std::string& name,
    ID3D12RootSignature* custom_root_signature,
    const ShaderConfig& shader_config,
    const RenderStateConfig& render_state = RenderStateConfig::Default());

  // Get existing material by name
  Material* GetMaterial(const std::string& name);

  // Root Signature Helpers
  // Get the shared/standard root signature (used by most materials)
  ID3D12RootSignature* GetSharedRootSignature() const {
    return shared_root_signature_.Get();
  }

  // Create a standard root signature (same as shared RS)
  // Useful when you want to create multiple materials with standard layout
  ComPtr<ID3D12RootSignature> CreateStandardRootSignature();

  // Create a terrain-specific root signature
  // Example: Add extra CBV for terrain-specific data
  ComPtr<ID3D12RootSignature> CreateTerrainRootSignature();

  // Create a water-specific root signature
  // Example: Add extra samplers for flow maps, caustics, etc
  ComPtr<ID3D12RootSignature> CreateWaterRootSignature();

  // ========================================
  // Default Materials
  // ========================================

  // Get default materials
  Material* GetDefaultOpaque() {
    return GetMaterial("Default_Opaque");
  }

  Material* GetDefaultTransparent() {
    return GetMaterial("Default_Transparent");
  }

  Material* GetDefaultUI() {
    return GetMaterial("Default_UI");
  }

 private:
  ID3D12Device* device_ = nullptr;
  ComPtr<ID3D12RootSignature> shared_root_signature_;
  std::unordered_map<std::string, std::unique_ptr<Material>> materials_;
  std::unordered_map<std::wstring, ComPtr<ID3DBlob>> shader_cache_;

  // Helper to load and cache shader
  ID3DBlob* LoadShader(const std::wstring& path);

  // Create the shared root signature that most materials use
  bool CreateSharedRootSignature();

  // Create default materials (Opaque, Transparent, UI)
  void CreateDefaultMaterials();

  // ========================================
  // Sort Key Generation
  // ========================================

  // Generate a 64-bit sort key from RS and PSO pointers
  // High 32 bits: RS hash, Low 32 bits: PSO hash
  uint64_t GenerateSortKey(ID3D12RootSignature* rs, ID3D12PipelineState* pso) const {
    uint32_t rs_hash = HashPointer(rs);
    uint32_t pso_hash = HashPointer(pso);
    return (static_cast<uint64_t>(rs_hash) << 32) | pso_hash;
  }

  // Simple hash function for D3D12 object pointers
  // Uses pointer address bits to generate deterministic hash
  uint32_t HashPointer(void* ptr) const {
    // Use upper bits of pointer for better distribution
    uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
    // Mix bits to avoid clustering (pointers are often aligned)
    uint32_t hash = static_cast<uint32_t>(addr ^ (addr >> 32));
    hash ^= (hash >> 16);
    return hash;
  }
};
