#pragma once
#include <d3d12.h>

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

#include "Core/types.h"
#include "shader_registry.h"

class ShaderManager {
 public:
  ShaderManager() = default;
  ~ShaderManager() = default;

  bool Initialize(ID3D12Device* device);

  const ShaderRegistry::ShaderMetadata& GetShaderMetadata(ShaderId id) const;

  std::string_view GetShaderName(ShaderId id) const;

  // Root Signature
  ID3D12RootSignature* GetRootSignature(Graphics::RSPreset preset) const;
  ID3D12RootSignature* GetRootSignature(ShaderId shader_id) const;

  template <typename ShaderType>
  ID3D12RootSignature* GetRootSignature() const {
    return GetRootSignature(ShaderType::RS_PRESET);
  }

  // Shader bytecode
  ID3DBlob* GetVertexShader(ShaderId id);
  ID3DBlob* GetPixelShader(ShaderId id);

  template <typename ShaderType>
  ID3DBlob* GetVertexShader() {
    return GetVertexShader(ShaderType::ID);
  }

  template <typename ShaderType>
  ID3DBlob* GetPixelShader() {
    return GetPixelShader(ShaderType::ID);
  }

  // Preload shaders to avoid runtime loading
  void PreloadShaders(const std::vector<ShaderId>& shader_ids);

 private:
  ID3D12Device* device_ = nullptr;

  // RS preset storage
  std::array<ComPtr<ID3D12RootSignature>, static_cast<size_t>(Graphics::RSPreset::Count)> rs_presets_;

  // Shader bytecode cache
  std::unordered_map<ShaderId, ComPtr<ID3DBlob>> vertex_shaders_;
  std::unordered_map<ShaderId, ComPtr<ID3DBlob>> pixel_shaders_;

  bool CreateRSPresets();
  bool CreateStandardRS();

  ID3DBlob* LoadShaderFromFile(const std::wstring& path);
};
