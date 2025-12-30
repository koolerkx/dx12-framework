#pragma once
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgiformat.h>
#include <memory>
#include <string>
#include <unordered_map>
#include "Core/types.h"
#include "Pipeline/pipeline_state_builder.h"
#include "Pipeline/root_signature_builder.h"
#include "Pipeline/sampler_builder.h"
#include "material.h"
#include "root_parameter_slots.h"
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

  // Create a material with specific shader config and render state
  Material* CreateMaterial(const std::string& name,
    const ShaderConfig& shader_config,
    const RenderStateConfig& render_state = RenderStateConfig::Default());

  // Get existing material by name
  Material* GetMaterial(const std::string& name);

  // Get the shared root signature (all materials currently use the same one)
  ID3D12RootSignature* GetSharedRootSignature() const {
    return shared_root_signature_.Get();
  }

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
  std::unordered_map<std::wstring, ComPtr<ID3DBlob>> shader_cache_;  // Cache compiled shaders
  
  uint32_t next_sort_key_ = 0;  // Auto-increment sort keys

  // Helper to load and cache shader
  ID3DBlob* LoadShader(const std::wstring& path);

  // Create the shared root signature that all materials use
  bool CreateSharedRootSignature();

  // Create default materials
  void CreateDefaultMaterials();
};
