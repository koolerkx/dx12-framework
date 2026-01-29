#include "shader_manager.h"

#include <d3d12shader.h>
#include <d3dcompiler.h>

#include <iostream>

#include "Pipeline/root_signature_builder.h"

bool ShaderManager::Initialize(ID3D12Device* device) {
  if (!device) {
    std::cerr << "[ShaderManager] Invalid device" << std::endl;
    return false;
  }

  device_ = device;

  if (!CreateRSPresets()) {
    std::cerr << "[ShaderManager] Failed to create root signature presets" << std::endl;
    return false;
  }

  std::cout << "[ShaderManager] Initialized with " << static_cast<size_t>(Graphics::RSPreset::Count) << " RS presets" << std::endl;
  return true;
}

const ShaderRegistry::ShaderMetadata& ShaderManager::GetShaderMetadata(Graphics::ShaderID id) const {
  return ShaderRegistry::GetMetadata(id);
}

Graphics::ShaderFamily ShaderManager::GetFamily(Graphics::ShaderID id) const {
  return ShaderRegistry::GetFamily(id);
}

std::string_view ShaderManager::GetShaderName(Graphics::ShaderID id) const {
  return ShaderRegistry::GetName(id);
}

ID3D12RootSignature* ShaderManager::GetRootSignature(Graphics::RSPreset preset) const {
  size_t index = static_cast<size_t>(preset);
  if (index >= rs_presets_.size()) {
    return nullptr;
  }
  return rs_presets_[index].Get();
}

ID3D12RootSignature* ShaderManager::GetRootSignature(Graphics::ShaderID shader_id) const {
  Graphics::ShaderFamily family = GetFamily(shader_id);
  return GetRootSignature(family);
}

ID3D12RootSignature* ShaderManager::GetRootSignature(Graphics::ShaderFamily family) const {
  Graphics::RSPreset preset = ShaderRegistry::GetRSPreset(family);
  return GetRootSignature(preset);
}

ID3DBlob* ShaderManager::GetVertexShader(Graphics::ShaderID id) {
  // Check cache first
  auto it = vertex_shaders_.find(id);
  if (it != vertex_shaders_.end()) {
    return it->second.Get();
  }

  // Load from file
  const ShaderRegistry::ShaderMetadata& metadata = GetShaderMetadata(id);
  ID3DBlob* blob = LoadShaderFromFile(std::wstring(metadata.vs_path));

  if (blob) {
    vertex_shaders_[id] = blob;
  }

  return blob;
}

ID3DBlob* ShaderManager::GetPixelShader(Graphics::ShaderID id) {
  // Check cache first
  auto it = pixel_shaders_.find(id);
  if (it != pixel_shaders_.end()) {
    return it->second.Get();
  }

  // Load from file
  const ShaderRegistry::ShaderMetadata& metadata = GetShaderMetadata(id);
  ID3DBlob* blob = LoadShaderFromFile(std::wstring(metadata.ps_path));

  if (blob) {
    pixel_shaders_[id] = blob;
  }

  return blob;
}

void ShaderManager::PreloadShaders(const std::vector<Graphics::ShaderID>& shader_ids) {
  std::cout << "[ShaderManager] Preloading " << shader_ids.size() << " shaders..." << std::endl;

  for (Graphics::ShaderID id : shader_ids) {
    GetVertexShader(id);
    GetPixelShader(id);
  }

  std::cout << "[ShaderManager] Preload complete" << std::endl;
}

bool ShaderManager::CreateRSPresets() {
  // Create all RS presets
  if (!CreateStandardRS()) {
    return false;
  }

  // Deferred and Compute RS are placeholders for future expansion
  // For now, they can use the same RS as Standard
  rs_presets_[static_cast<size_t>(Graphics::RSPreset::Deferred)] = rs_presets_[static_cast<size_t>(Graphics::RSPreset::Standard)];
  rs_presets_[static_cast<size_t>(Graphics::RSPreset::Compute)] = rs_presets_[static_cast<size_t>(Graphics::RSPreset::Standard)];

  return true;
}

bool ShaderManager::CreateStandardRS() {
  try {
    constexpr uint32_t SRV_CAPACITY = 4096;
    constexpr uint32_t SAMPLER_CAPACITY = 2048;

    rs_presets_[static_cast<size_t>(Graphics::RSPreset::Standard)] = RootSignatureBuilder()
                                                                       .AllowInputLayout()
                                                                       // Fixed CBV slots
                                                                       .AddRootCBV(0, 0)  // Slot 0: Frame (b0)
                                                                       .AddRootCBV(1, 0)  // Slot 1: Object (b1)
                                                                       .AddRootCBV(2, 0)  // Slot 2: Light (b2)
                                                                       // Material data (texture indices, etc)
                                                                       .Add32BitConstants(4, 3, 0)  // Slot 3: MaterialData (b3)
                                                                       // Global bindless texture array
                                                                       .AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                                                                         SRV_CAPACITY,
                                                                         0,                            // register(t0)
                                                                         1,                            // space1
                                                                         D3D12_SHADER_VISIBILITY_ALL)  // Slot 4: GlobalSRVs
                                                                       // Bindless sampler array
                                                                       .AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
                                                                         SAMPLER_CAPACITY,
                                                                         0,                              // register(s0)
                                                                         0,                              // space0
                                                                         D3D12_SHADER_VISIBILITY_PIXEL)  // Slot 5: Samplers
                                                                       .Build(device_);

    if (!rs_presets_[static_cast<size_t>(Graphics::RSPreset::Standard)]) {
      std::cerr << "[ShaderManager] Failed to create Standard RS" << std::endl;
      return false;
    }

    std::cout << "[ShaderManager] Created Standard RS preset" << std::endl;
    return true;
  } catch (const std::exception& e) {
    std::cerr << "[ShaderManager] Exception creating Standard RS: " << e.what() << std::endl;
    return false;
  }
}

bool ShaderManager::CreateDeferredRS() {
  // TODO: Implement Deferred RS with additional render targets
  // For now, use Standard RS as fallback
  return true;
}

bool ShaderManager::CreateComputeRS() {
  // TODO: Implement Compute RS (no input layout, different shader visibility)
  // For now, use Standard RS as fallback
  return true;
}

ID3DBlob* ShaderManager::LoadShaderFromFile(const std::wstring& path) {
  ComPtr<ID3DBlob> blob;
  HRESULT hr = D3DReadFileToBlob(path.c_str(), &blob);

  if (FAILED(hr)) {
    std::wcerr << L"[ShaderManager] Failed to load shader: " << path << std::endl;
    return nullptr;
  }

  return blob.Detach();
}
