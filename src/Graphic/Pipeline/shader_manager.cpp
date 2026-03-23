#include "shader_manager.h"

#include <d3d12shader.h>
#include <d3dcompiler.h>

#include "Core/utils.h"
#include "Framework/Logging/logger.h"
#include "Pipeline/root_signature_builder.h"

bool ShaderManager::Initialize(ID3D12Device* device) {
  if (!device) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Resource, Logger::Here(), "[ShaderManager] Invalid device");
    return false;
  }

  device_ = device;

  if (!CreateRSPresets()) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Resource, Logger::Here(), "[ShaderManager] Failed to create root signature presets");
    return false;
  }

  Logger::LogFormat(LogLevel::Info,
    LogCategory::Resource,
    Logger::Here(),
    "[ShaderManager] Initialized with {} RS presets",
    static_cast<size_t>(Graphics::RSPreset::Count));
  return true;
}

const ShaderRegistry::ShaderMetadata& ShaderManager::GetShaderMetadata(ShaderId id) const {
  return ShaderRegistry::GetMetadata(id);
}

std::string_view ShaderManager::GetShaderName(ShaderId id) const {
  return ShaderRegistry::GetName(id);
}

ID3D12RootSignature* ShaderManager::GetRootSignature(Graphics::RSPreset preset) const {
  size_t index = static_cast<size_t>(preset);
  if (index >= rs_presets_.size()) {
    return nullptr;
  }
  return rs_presets_[index].Get();
}

ID3D12RootSignature* ShaderManager::GetRootSignature(ShaderId shader_id) const {
  Graphics::RSPreset preset = ShaderRegistry::GetRSPreset(shader_id);
  return GetRootSignature(preset);
}

ID3DBlob* ShaderManager::GetVertexShader(ShaderId id) {
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

ID3DBlob* ShaderManager::GetPixelShader(ShaderId id) {
  const ShaderRegistry::ShaderMetadata& metadata = GetShaderMetadata(id);
  if (metadata.ps_path.empty()) {
    return nullptr;
  }

  // Check cache first
  auto it = pixel_shaders_.find(id);
  if (it != pixel_shaders_.end()) {
    return it->second.Get();
  }

  // Load from file
  ID3DBlob* blob = LoadShaderFromFile(std::wstring(metadata.ps_path));

  if (blob) {
    pixel_shaders_[id] = blob;
  }

  return blob;
}

void ShaderManager::PreloadShaders(const std::vector<ShaderId>& shader_ids) {
  Logger::LogFormat(LogLevel::Info, LogCategory::Resource, Logger::Here(), "[ShaderManager] Preloading {} shaders...", shader_ids.size());

  for (ShaderId id : shader_ids) {
    GetVertexShader(id);
    GetPixelShader(id);
  }

  Logger::LogFormat(LogLevel::Info, LogCategory::Resource, Logger::Here(), "[ShaderManager] Preload complete");
}

bool ShaderManager::CreateRSPresets() {
  return CreateStandardRS();
}

bool ShaderManager::CreateStandardRS() {
  try {
    constexpr uint32_t SRV_CAPACITY = 4096;
    constexpr uint32_t SAMPLER_CAPACITY = 2048;

    rs_presets_[static_cast<size_t>(Graphics::RSPreset::Standard)] =
      RootSignatureBuilder()
        .AllowInputLayout()
        // Fixed CBV slots
        .AddRootCBV(0, 0)  // Slot 0: Frame (b0)
        .AddRootCBV(1, 0)  // Slot 1: Object (b1)
        .AddRootCBV(2, 0)  // Slot 2: Light (b2)
        // Material data (texture indices, etc)
        .AddRootCBV(3, 0)  // Slot 3: Material (b3)
        // Shadow mapping data
        .AddRootCBV(4, 0)  // Slot 4: Shadow (b4)
        // Global bindless texture array
        .AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
          SRV_CAPACITY,
          0,                            // register(t0)
          1,                            // space1
          D3D12_SHADER_VISIBILITY_ALL)  // Slot 5: GlobalSRVs
        // Bindless sampler array
        .AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
          SAMPLER_CAPACITY,
          0,                              // register(s0)
          0,                              // space0
          D3D12_SHADER_VISIBILITY_PIXEL)  // Slot 6: Samplers
        // Point light structured buffer
        .AddRootSRV(0, 2)  // Slot 7: PointLights (t0, space2)
        // Instance data structured buffer
        .AddRootSRV(0, 3)  // Slot 8: InstanceBuffer (t0, space3)
        .AddRootCBV(5, 0)  // Slot 9: Custom (b5)
        .AddRootSRV(0, 4)  // Slot 10: MeshDescriptors (t0, space4)
        .AddRootSRV(0, 5)  // Slot 11: MaterialDescriptors (t0, space5)
        .AddStaticSampler(SamplerPresets::CreateComparisonSampler(0).SetRegisterSpace(2))
        .Build(device_);

    if (!rs_presets_[static_cast<size_t>(Graphics::RSPreset::Standard)]) {
      Logger::LogFormat(LogLevel::Error, LogCategory::Resource, Logger::Here(), "[ShaderManager] Failed to create Standard RS");
      return false;
    }

    Logger::LogFormat(LogLevel::Info, LogCategory::Resource, Logger::Here(), "[ShaderManager] Created Standard RS preset");
    return true;
  } catch (const std::exception& e) {
    Logger::LogFormat(
      LogLevel::Error, LogCategory::Resource, Logger::Here(), "[ShaderManager] Exception creating Standard RS: {}", e.what());
    return false;
  }
}

ID3DBlob* ShaderManager::LoadShaderFromFile(const std::wstring& path) {
  ComPtr<ID3DBlob> blob;
  HRESULT hr = D3DReadFileToBlob(path.c_str(), &blob);

  if (FAILED(hr)) {
    Logger::LogFormat(
      LogLevel::Error, LogCategory::Resource, Logger::Here(), "[ShaderManager] Failed to load shader: {}", utils::wstring_to_utf8(path));
    return nullptr;
  }

  return blob.Detach();
}
