#include "material_manager.h"
#include <iostream>

bool MaterialManager::Initialize(ID3D12Device* device) {
  if (!device) {
    std::cerr << "[MaterialManager] Invalid device" << std::endl;
    return false;
  }

  device_ = device;

  if (!CreateSharedRootSignature()) {
    std::cerr << "[MaterialManager] Failed to create shared root signature" << std::endl;
    return false;
  }

  CreateDefaultMaterials();
  return true;
}

bool MaterialManager::CreateSharedRootSignature() {
  try {
    constexpr uint32_t SRV_CAPACITY = 4096;

    shared_root_signature_ = RootSignatureBuilder()
                               .AllowInputLayout()
                               // Fixed CBV slots
                               .AddRootCBV(0, 0)  // RootSlot::ConstantBuffer::Frame
                               .AddRootCBV(1, 0)  // RootSlot::ConstantBuffer::Object
                               .AddRootCBV(2, 0)  // RootSlot::ConstantBuffer::Light (future)
                               // Material data (texture indices, etc)
                               .Add32BitConstants(4, 3, 0)  // RootSlot::Constants::MaterialData
                               // Global bindless texture array
                               .AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                                 SRV_CAPACITY,
                                 0,  // register(t0)
                                 1,  // space1
                                 D3D12_SHADER_VISIBILITY_ALL)  // RootSlot::DescriptorTable::GlobalSRVs
                               // Static samplers
                               .AddStaticSampler(SamplerPresets::CreatePointSampler(0, D3D12_TEXTURE_ADDRESS_MODE_WRAP))
                               .AddStaticSampler(SamplerPresets::CreateLinearSampler(1, D3D12_TEXTURE_ADDRESS_MODE_WRAP))
                               .AddStaticSampler(
                                 SamplerPresets::CreateAnisotropicSampler(2, 16, D3D12_TEXTURE_ADDRESS_MODE_WRAP))
                               .AddStaticSampler(SamplerPresets::CreatePointSampler(3, D3D12_TEXTURE_ADDRESS_MODE_CLAMP))
                               .AddStaticSampler(SamplerPresets::CreateLinearSampler(4, D3D12_TEXTURE_ADDRESS_MODE_CLAMP))
                               .Build(device_);

    if (!shared_root_signature_) {
      return false;
    }

    return true;
  } catch (const std::exception& e) {
    std::cerr << "[MaterialManager] Exception creating root signature: " << e.what() << std::endl;
    return false;
  }
}

void MaterialManager::CreateDefaultMaterials() {
  // Default Opaque Material (for 3D objects)
  {
    ShaderConfig config = ShaderPresets::CreateBasic3D();
    RenderStateConfig render_state = RenderStateConfig::Default();
    CreateMaterial("Default_Opaque", config, render_state);
  }

  // Default Transparent Material
  {
    ShaderConfig config = ShaderPresets::CreateBasic3D();
    RenderStateConfig render_state = RenderStateConfig::Transparent();
    CreateMaterial("Default_Transparent", config, render_state);
  }

  // Default UI Material (for sprites, UI elements)
  {
    ShaderConfig config = ShaderPresets::CreateSprite();
    RenderStateConfig render_state = RenderStateConfig::UI();
    CreateMaterial("Default_UI", config, render_state);
  }
}

Material* MaterialManager::CreateMaterial(const std::string& name,
  const ShaderConfig& shader_config,
  const RenderStateConfig& render_state) {
  
  // Check if material already exists
  if (materials_.find(name) != materials_.end()) {
    std::cerr << "[MaterialManager] Material '" << name << "' already exists" << std::endl;
    return materials_[name].get();
  }

  // Load shaders
  ID3DBlob* vs_blob = LoadShader(shader_config.vs_path);
  ID3DBlob* ps_blob = LoadShader(shader_config.ps_path);

  if (!vs_blob || !ps_blob) {
    std::cerr << "[MaterialManager] Failed to load shaders for material '" << name << "'" << std::endl;
    return nullptr;
  }

  // Create PSO
  try {
    ComPtr<ID3D12PipelineState> pso = PipelineStateBuilder()
                                        .SetRootSignature(shared_root_signature_.Get())
                                        .SetVertexShader(vs_blob)
                                        .SetPixelShader(ps_blob)
                                        .SetInputLayout(shader_config.input_layout)
                                        .SetRenderTargetFormat(render_state.rtv_format)
                                        .SetDepthStencilFormat(render_state.dsv_format)
                                        .SetCullMode(render_state.cull_mode)
                                        .SetDepthTest(render_state.depth_test_enabled)
                                        .SetDepthWrite(render_state.depth_write_enabled)
                                        .SetBlendMode(render_state.blend_mode)
                                        .Build(device_);

    if (!pso) {
      std::cerr << "[MaterialManager] Failed to create PSO for material '" << name << "'" << std::endl;
      return nullptr;
    }

    // Create material with auto-incremented sort key
    auto material = std::make_unique<Material>(name, shared_root_signature_.Get(), pso.Get(), next_sort_key_++);

    Material* material_ptr = material.get();
    materials_[name] = std::move(material);

    std::cout << "[MaterialManager] Created material '" << name << "' with sort key " << material_ptr->GetSortKey()
              << std::endl;

    return material_ptr;
  } catch (const std::exception& e) {
    std::cerr << "[MaterialManager] Exception creating material '" << name << "': " << e.what() << std::endl;
    return nullptr;
  }
}

Material* MaterialManager::GetMaterial(const std::string& name) {
  auto it = materials_.find(name);
  if (it != materials_.end()) {
    return it->second.get();
  }
  return nullptr;
}

ID3DBlob* MaterialManager::LoadShader(const std::wstring& path) {
  // Check cache first
  auto it = shader_cache_.find(path);
  if (it != shader_cache_.end()) {
    return it->second.Get();
  }

  // Load shader
  ComPtr<ID3DBlob> blob;
  HRESULT hr = D3DReadFileToBlob(path.c_str(), &blob);
  if (FAILED(hr)) {
    std::wcerr << L"[MaterialManager] Failed to load shader: " << path << std::endl;
    return nullptr;
  }

  // Cache it
  shader_cache_[path] = blob;
  return blob.Get();
}
