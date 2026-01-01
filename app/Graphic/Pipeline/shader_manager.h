#pragma once
#include <d3d12.h>

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

#include "Core/types.h"
#include "shader_metadata.h"

class ShaderManager {
 public:
  ShaderManager() = default;
  ~ShaderManager() = default;

  // Create RS Preset
  bool Initialize(ID3D12Device* device);

  const ShaderRegistry::ShaderMetadata& GetShaderMetadata(Graphics::ShaderID id) const;

  Graphics::ShaderFamily GetFamily(Graphics::ShaderID id) const;

  std::string_view GetShaderName(Graphics::ShaderID id) const;

  // RS
  ID3D12RootSignature* GetRootSignature(Graphics::RSPreset preset) const;

  ID3D12RootSignature* GetRootSignature(Graphics::ShaderID shader_id) const;

  ID3D12RootSignature* GetRootSignature(Graphics::ShaderFamily family) const;

  // Shader
  ID3DBlob* GetVertexShader(Graphics::ShaderID id);
  ID3DBlob* GetPixelShader(Graphics::ShaderID id);

  // Prevent runtime shader
  void PreloadShaders(const std::vector<Graphics::ShaderID>& shader_ids);

 private:
  ID3D12Device* device_ = nullptr;

  // RS preset storage
  std::array<ComPtr<ID3D12RootSignature>, static_cast<size_t>(Graphics::RSPreset::Count)> rs_presets_;

  // Shader bytecode cache
  std::unordered_map<Graphics::ShaderID, ComPtr<ID3DBlob>> vertex_shaders_;
  std::unordered_map<Graphics::ShaderID, ComPtr<ID3DBlob>> pixel_shaders_;

  bool CreateRSPresets();
  bool CreateStandardRS();
  bool CreateDeferredRS();
  bool CreateComputeRS();

  ID3DBlob* LoadShaderFromFile(const std::wstring& path);
};
