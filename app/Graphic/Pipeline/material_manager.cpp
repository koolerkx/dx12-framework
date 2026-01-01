#include "material_manager.h"

#include <iostream>
#include <sstream>

#include "Game/Component/render_settings.h"
#include "Pipeline/root_signature_builder.h"
#include "Pipeline/sampler_builder.h"

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
    constexpr uint32_t SAMPLER_CAPACITY = 2048;

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
                                 0,                            // register(t0)
                                 1,                            // space1
                                 D3D12_SHADER_VISIBILITY_ALL)  // RootSlot::DescriptorTable::GlobalSRVs
                               // Bindless sampler array
                               .AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
                                 SAMPLER_CAPACITY,
                                 0,                              // register(s0)
                                 0,                              // space0
                                 D3D12_SHADER_VISIBILITY_PIXEL)  // RootSlot::DescriptorTable::Samplers
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

  // Instanced UI Material (for batched text/sprite rendering with hardware instancing)
  {
    ShaderConfig config = ShaderPresets::CreateSpriteInstancedUI();
    RenderStateConfig render_state = RenderStateConfig::UI();
    CreateMaterial("Instanced_UI", config, render_state);
  }

  // World Text Material (no culling for single-sided quads)
  {
    ShaderConfig config = ShaderPresets::CreateBasic3D();
    RenderStateConfig render_state = RenderStateConfig::Default();
    render_state.cull_mode = D3D12_CULL_MODE_NONE;
    CreateMaterial("World_Text", config, render_state);
  }

  // Instanced World Text Material (for batched world text rendering with hardware instancing)
  {
    ShaderConfig config = ShaderPresets::CreateSpriteInstancedWorld();
    RenderStateConfig render_state = RenderStateConfig::Default();
    render_state.cull_mode = D3D12_CULL_MODE_NONE;  // No culling for single-sided quads
    CreateMaterial("Instanced_World_Text", config, render_state);
  }

  // Instanced World Text Transparent Material (for transparent world text with alpha blending)
  {
    ShaderConfig config = ShaderPresets::CreateSpriteInstancedWorldTransparent();
    RenderStateConfig render_state = RenderStateConfig::Transparent();
    render_state.cull_mode = D3D12_CULL_MODE_NONE;  // No culling for single-sided quads
    CreateMaterial("Instanced_World_Text_Transparent", config, render_state);
  }

  // Debug Line Material
  {
    ShaderConfig config;
    config.vs_path = L"Content/shaders/debug_line.vs.cso";
    config.ps_path = L"Content/shaders/debug_line.ps.cso";

    // Line vertex input layout
    config.input_layout = {{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

    RenderStateConfig render_state = RenderStateConfig::Default();
    render_state.primitive_topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    render_state.depth_test_enabled = true;
    render_state.depth_write_enabled = false;       // Lines don't write depth
    render_state.cull_mode = D3D12_CULL_MODE_NONE;  // Don't cull lines

    CreateMaterial("Debug_Line", config, render_state);
  }
}

// Create material with shared RS
Material* MaterialManager::CreateMaterial(
  const std::string& name, const ShaderConfig& shader_config, const RenderStateConfig& render_state) {
  // Delegate to the custom RS version, using shared RS
  return CreateMaterial(name, shared_root_signature_.Get(), shader_config, render_state);
}

// Create material with custom RS
Material* MaterialManager::CreateMaterial(const std::string& name,
  ID3D12RootSignature* custom_root_signature,
  const ShaderConfig& shader_config,
  const RenderStateConfig& render_state) {
  // Check if material already exists
  if (materials_.find(name) != materials_.end()) {
    std::cerr << "[MaterialManager] Material '" << name << "' already exists" << std::endl;
    return materials_[name].get();
  }

  // Validate root signature
  if (!custom_root_signature) {
    std::cerr << "[MaterialManager] Invalid root signature for material '" << name << "'" << std::endl;
    return nullptr;
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
                                        .SetRootSignature(custom_root_signature)
                                        .SetVertexShader(vs_blob)
                                        .SetPixelShader(ps_blob)
                                        .SetInputLayout(shader_config.input_layout)
                                        .SetRenderTargetFormat(render_state.rtv_format)
                                        .SetDepthStencilFormat(render_state.dsv_format)
                                        .SetCullMode(render_state.cull_mode)
                                        .SetDepthTest(render_state.depth_test_enabled)
                                        .SetDepthWrite(render_state.depth_write_enabled)
                                        .SetPrimitiveTopology(render_state.primitive_topology)
                                        .SetBlendMode(render_state.blend_mode)
                                        .Build(device_);

    if (!pso) {
      std::cerr << "[MaterialManager] Failed to create PSO for material '" << name << "'" << std::endl;
      return nullptr;
    }

    // Generate 64-bit sort key: [RS hash | PSO hash]
    uint64_t sort_key = GenerateSortKey(custom_root_signature, pso.Get());

    // Create material
    auto material = std::make_unique<Material>(name, custom_root_signature, pso.Get(), sort_key);

    Material* material_ptr = material.get();
    materials_[name] = std::move(material);

    std::cout << "[MaterialManager] Created material '" << name << "' RS key: 0x" << std::hex << material_ptr->GetRootSignatureKey()
              << " PSO key: 0x" << material_ptr->GetPSOKey() << std::dec << std::endl;

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

// Root Signature Creation Helpers
ComPtr<ID3D12RootSignature> MaterialManager::CreateStandardRootSignature() {
  // Returns a reference to the shared root signature (not a new instance)
  // All standard materials share the same root signature for optimal batching
  return shared_root_signature_;
}

ComPtr<ID3D12RootSignature> MaterialManager::CreateTerrainRootSignature() {
  try {
    constexpr uint32_t SRV_CAPACITY = 4096;

    // Terrain RS: Standard layout with extra CBV after descriptor table
    // This keeps the SRV table at the same slot (4) for consistency
    auto terrain_rs = RootSignatureBuilder()
                        .AllowInputLayout()
                        // Fixed CBV slots (same as standard)
                        .AddRootCBV(0, 0)            // Slot 0: Frame (b0)
                        .AddRootCBV(1, 0)            // Slot 1: Object (b1)
                        .AddRootCBV(2, 0)            // Slot 2: Light (b2)
                        .Add32BitConstants(4, 3, 0)  // Slot 3: Material data (b3)
                        // Global bindless texture array (same slot as standard)
                        .AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                          SRV_CAPACITY,
                          0,                            // register(t0)
                          1,                            // space1
                          D3D12_SHADER_VISIBILITY_ALL)  // Slot 4: SRV Table
                        .AddRootCBV(4, 0)               // Slot 5: Terrain-specific data (b4)
                        // Static samplers (same as standard)
                        .AddStaticSampler(SamplerPresets::CreatePointSampler(0, D3D12_TEXTURE_ADDRESS_MODE_WRAP))
                        .AddStaticSampler(SamplerPresets::CreateLinearSampler(1, D3D12_TEXTURE_ADDRESS_MODE_WRAP))
                        .AddStaticSampler(SamplerPresets::CreateAnisotropicSampler(2, 16, D3D12_TEXTURE_ADDRESS_MODE_WRAP))
                        .AddStaticSampler(SamplerPresets::CreatePointSampler(3, D3D12_TEXTURE_ADDRESS_MODE_CLAMP))
                        .AddStaticSampler(SamplerPresets::CreateLinearSampler(4, D3D12_TEXTURE_ADDRESS_MODE_CLAMP))
                        .Build(device_);

    return terrain_rs;
  } catch (const std::exception& e) {
    std::cerr << "[MaterialManager] Exception creating terrain root signature: " << e.what() << std::endl;
    return nullptr;
  }
}

ComPtr<ID3D12RootSignature> MaterialManager::CreateWaterRootSignature() {
  try {
    constexpr uint32_t SRV_CAPACITY = 4096;

    // Water RS: Standard layout with extra CBV after descriptor table
    // This keeps the SRV table at the same slot (4) for consistency
    auto water_rs = RootSignatureBuilder()
                      .AllowInputLayout()
                      // Fixed CBV slots (same as standard)
                      .AddRootCBV(0, 0)            // Slot 0: Frame (b0)
                      .AddRootCBV(1, 0)            // Slot 1: Object (b1)
                      .AddRootCBV(2, 0)            // Slot 2: Light (b2)
                      .Add32BitConstants(4, 3, 0)  // Slot 3: Material data (b3)
                      // Global bindless texture array (same slot as standard)
                      .AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                        SRV_CAPACITY,
                        0,                            // register(t0)
                        1,                            // space1
                        D3D12_SHADER_VISIBILITY_ALL)  // Slot 4: SRV Table
                      .AddRootCBV(4, 0)               // Slot 5: Water-specific data (b4: wave params, etc)
                      // Static samplers (standard + water-specific)
                      .AddStaticSampler(SamplerPresets::CreatePointSampler(0, D3D12_TEXTURE_ADDRESS_MODE_WRAP))
                      .AddStaticSampler(SamplerPresets::CreateLinearSampler(1, D3D12_TEXTURE_ADDRESS_MODE_WRAP))
                      .AddStaticSampler(SamplerPresets::CreateAnisotropicSampler(2, 16, D3D12_TEXTURE_ADDRESS_MODE_WRAP))
                      .AddStaticSampler(SamplerPresets::CreatePointSampler(3, D3D12_TEXTURE_ADDRESS_MODE_CLAMP))
                      .AddStaticSampler(SamplerPresets::CreateLinearSampler(4, D3D12_TEXTURE_ADDRESS_MODE_CLAMP))
                      // Extra sampler for flow map
                      .AddStaticSampler(SamplerPresets::CreateLinearSampler(5, D3D12_TEXTURE_ADDRESS_MODE_WRAP))
                      .Build(device_);

    return water_rs;
  } catch (const std::exception& e) {
    std::cerr << "[MaterialManager] Exception creating water root signature: " << e.what() << std::endl;
    return nullptr;
  }
}

// Dynamic Material System Implementation
Material* MaterialManager::GetOrCreateMaterial(const Rendering::RenderSettings& settings) {
  settings.Validate();
  uint32_t key = settings.GetCacheKey();

  // Read lock first (fast path)
  {
    std::shared_lock read_lock(cache_mutex_);
    if (auto it = pso_cache_.find(key); it != pso_cache_.end()) {
      it->second.last_used_frame = current_frame_;
      return it->second.material.get();
    }
  }

  // Write lock for creation (slow path)
  std::unique_lock write_lock(cache_mutex_);

  // Double-check after acquiring write lock
  if (auto it = pso_cache_.find(key); it != pso_cache_.end()) {
    it->second.last_used_frame = current_frame_;
    return it->second.material.get();
  }

  // Create new material
  CacheEntry entry;
  entry.material = std::make_unique<Material>(CreateMaterialInternal(settings));
  entry.last_used_frame = current_frame_;

  Material* ptr = entry.material.get();
  pso_cache_[key] = std::move(entry);

  return ptr;
}

Material MaterialManager::CreateMaterialInternal(const Rendering::RenderSettings& settings) {
  // Map Rendering::BlendMode to Graphics::BlendMode
  BlendMode blend_mode = BlendMode::AlphaBlend;  // Default
  switch (settings.blend_mode) {
    case Rendering::BlendMode::Opaque:
      blend_mode = BlendMode::Opaque;
      break;
    case Rendering::BlendMode::AlphaBlend:
      blend_mode = BlendMode::AlphaBlend;
      break;
    case Rendering::BlendMode::Additive:
      blend_mode = BlendMode::Additive;
      break;
    case Rendering::BlendMode::Premultiplied:
      blend_mode = BlendMode::Premultiplied;
      break;
  }

  // Select appropriate shader based on blend mode
  ShaderConfig shader_config;
  if (blend_mode == BlendMode::Opaque) {
    shader_config = ShaderPresets::CreateSpriteInstancedWorld();
  } else {
    shader_config = ShaderPresets::CreateSpriteInstancedWorldTransparent();
  }

  // Load shaders
  ID3DBlob* vs_blob = LoadShader(shader_config.vs_path);
  ID3DBlob* ps_blob = LoadShader(shader_config.ps_path);

  if (!vs_blob || !ps_blob) {
    // Return default material if shader loading fails
    return Material();
  }

  // Build render state
  RenderStateConfig render_state;
  render_state.blend_mode = blend_mode;
  render_state.depth_test_enabled = settings.depth_test;
  render_state.depth_write_enabled = settings.depth_write;
  render_state.cull_mode = settings.double_sided ? D3D12_CULL_MODE_NONE : D3D12_CULL_MODE_BACK;

  // Create PSO
  auto pso = PipelineStateBuilder()
               .SetRootSignature(shared_root_signature_.Get())
               .SetVertexShader(vs_blob)
               .SetPixelShader(ps_blob)
               .SetInputLayout(shader_config.input_layout)
               .SetBlendMode(render_state.blend_mode)
               .SetCullMode(render_state.cull_mode)
               .SetDepthTest(render_state.depth_test_enabled)
               .SetDepthWrite(render_state.depth_write_enabled)
               .SetRenderTargetFormat(render_state.rtv_format)
               .SetDepthStencilFormat(render_state.dsv_format)
               .SetPrimitiveTopology(render_state.primitive_topology)
               .Build(device_);

  if (!pso) {
    return Material();
  }

  // Generate name for debugging
  std::string name = GenerateMaterialName(settings);
  std::wstring wide_name(name.begin(), name.end());
  pso->SetName(wide_name.c_str());

  // Generate sort key
  uint64_t sort_key = GenerateSortKey(shared_root_signature_.Get(), pso.Get());

  return Material(name, shared_root_signature_.Get(), pso.Get(), sort_key);
}

std::string MaterialManager::GenerateMaterialName(const Rendering::RenderSettings& settings) const {
  std::ostringstream oss;
  oss << "Dynamic_";

  switch (settings.blend_mode) {
    case Rendering::BlendMode::Opaque:
      oss << "Opaque";
      break;
    case Rendering::BlendMode::AlphaBlend:
      oss << "Alpha";
      break;
    case Rendering::BlendMode::Additive:
      oss << "Add";
      break;
    case Rendering::BlendMode::Premultiplied:
      oss << "Premult";
      break;
  }

  if (settings.depth_test) {
    oss << "_Depth";
    if (settings.depth_write) {
      oss << "W";
    }
  }

  if (settings.double_sided) {
    oss << "_2Side";
  }

  oss << "_S" << static_cast<int>(settings.sampler_type);

  return oss.str();
}

// Update frame counter for LRU cache
void MaterialManager::OnFrameEnd() {
  current_frame_++;

  std::unique_lock lock(cache_mutex_);
  if (pso_cache_.size() > MAX_CACHE_SIZE) {
    EvictLRU();
  }
}

void MaterialManager::EvictLRU() {
  if (pso_cache_.empty()) {
    return;
  }

  auto oldest = std::min_element(
    pso_cache_.begin(), pso_cache_.end(), [](const auto& a, const auto& b) { return a.second.last_used_frame < b.second.last_used_frame; });

  pso_cache_.erase(oldest);
}
