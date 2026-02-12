#include "material_manager.h"

#include <sstream>

#include "Framework/Logging/logger.h"
#include "Game/Component/render_settings.h"
#include "Pipeline/pipeline_state_builder.h"
#include "Pipeline/shader_registry.h"

bool MaterialManager::Initialize(ID3D12Device* device, ShaderManager* shader_manager) {
  if (!device) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Resource, Logger::Here(), "[MaterialManager] Invalid device");
    return false;
  }
  if (!shader_manager) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Resource, Logger::Here(), "[MaterialManager] Invalid shader manager");
    return false;
  }

  device_ = device;
  shader_manager_ = shader_manager;
  return true;
}

Rendering::RenderSettings MaterialManager::GetDefaultSettings(Graphics::ShaderId shader_id) {
  Rendering::RenderSettings settings;

  if (shader_id == Graphics::DebugLineShader::ID) {
    settings.blend_mode = Rendering::BlendMode::AlphaBlend;
    settings.depth_test = true;
    settings.depth_write = false;
    settings.double_sided = true;
    settings.sampler_type = Rendering::SamplerType::LinearWrap;
    settings.render_target_format = Rendering::RenderTargetFormat::HDR;
  } else if (shader_id == Graphics::SpriteShader::ID) {
    settings.blend_mode = Rendering::BlendMode::AlphaBlend;
    settings.depth_test = false;
    settings.depth_write = false;
    settings.double_sided = false;
    settings.sampler_type = Rendering::SamplerType::LinearWrap;
    settings.render_target_format = Rendering::RenderTargetFormat::HDR;
  } else if (shader_id == Graphics::SpriteInstancedShader::ID) {
    settings = Rendering::RenderSettings::Opaque();
  } else if (shader_id == Graphics::Basic3DShader::ID) {
    settings = Rendering::RenderSettings::Opaque();
  } else {
    settings = Rendering::RenderSettings::Opaque();
  }

  return settings;
}

Material* MaterialManager::GetOrCreateMaterial(Graphics::ShaderId shader_id, const Rendering::RenderSettings& settings) {
  settings.Validate();
  uint64_t key = GenerateCacheKey(shader_id, settings);

  // Read lock first (fast path)
  {
    std::shared_lock read_lock(cache_mutex_);
    if (auto it = unified_cache_.find(key); it != unified_cache_.end()) {
      it->second.last_used_frame = current_frame_;
      return it->second.material.get();
    }
  }

  // Write lock for creation (slow path)
  std::unique_lock write_lock(cache_mutex_);

  // Double-check after acquiring write lock
  if (auto it = unified_cache_.find(key); it != unified_cache_.end()) {
    it->second.last_used_frame = current_frame_;
    return it->second.material.get();
  }

  // Create new material
  CacheEntry entry;
  entry.material = std::make_unique<Material>(CreateMaterialInternal(shader_id, settings));
  entry.last_used_frame = current_frame_;

  if (!entry.material || !entry.material->IsValid()) {
    Logger::LogFormat(
      LogLevel::Error, LogCategory::Resource, Logger::Here(), "[MaterialManager] Failed to create material for ShaderId: {}", shader_id);
    entry.material.reset();
    unified_cache_[key] = std::move(entry);
    return nullptr;
  }

  Material* ptr = entry.material.get();
  unified_cache_[key] = std::move(entry);

  return ptr;
}

Material MaterialManager::CreateMaterialInternal(Graphics::ShaderId shader_id, const Rendering::RenderSettings& settings) {
  // Get shader metadata and bytecode from ShaderManager
  const ShaderRegistry::ShaderMetadata& metadata = shader_manager_->GetShaderMetadata(shader_id);
  ID3DBlob* vs_blob = shader_manager_->GetVertexShader(shader_id);
  ID3DBlob* ps_blob = shader_manager_->GetPixelShader(shader_id);

  if (!vs_blob || !ps_blob) {
    Logger::LogFormat(
      LogLevel::Error, LogCategory::Resource, Logger::Here(), "[MaterialManager] Failed to load shaders for ShaderId: {}", shader_id);
    return Material();
  }

  // Get Root Signature from ShaderManager
  ID3D12RootSignature* rs = shader_manager_->GetRootSignature(shader_id);
  if (!rs) {
    Logger::LogFormat(
      LogLevel::Error, LogCategory::Resource, Logger::Here(), "[MaterialManager] Failed to get Root Signature for ShaderId: {}", shader_id);
    return Material();
  }

  // Map Rendering::BlendMode to Graphics::BlendMode
  BlendMode blend_mode = BlendMode::AlphaBlend;
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

  // Determine RTV format based on RenderSettings
  DXGI_FORMAT rtv_format;
  switch (settings.render_target_format) {
    case Rendering::RenderTargetFormat::HDR:
      rtv_format = DXGI_FORMAT_R16G16B16A16_FLOAT;
      break;
    case Rendering::RenderTargetFormat::SDR:
    default:
      rtv_format = DXGI_FORMAT_R8G8B8A8_UNORM;
      break;
  }
  constexpr DXGI_FORMAT dsv_format = DXGI_FORMAT_D32_FLOAT;
  D3D12_CULL_MODE cull_mode = settings.double_sided ? D3D12_CULL_MODE_NONE : D3D12_CULL_MODE_BACK;
  D3D12_PRIMITIVE_TOPOLOGY_TYPE primitive_topology = metadata.render_hints.topology;

  // Create PSO
  std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout_vec(metadata.input_layout.begin(), metadata.input_layout.end());
  auto pso = PipelineStateBuilder()
               .SetRootSignature(rs)
               .SetVertexShader(vs_blob)
               .SetPixelShader(ps_blob)
               .SetInputLayout(input_layout_vec)
               .SetBlendMode(blend_mode)
               .SetCullMode(cull_mode)
               .SetDepthTest(settings.depth_test)
               .SetDepthWrite(settings.depth_write)
               .SetRenderTargetFormat(rtv_format)
               .SetDepthStencilFormat(dsv_format)
               .SetPrimitiveTopology(primitive_topology)
               .Build(device_);

  if (!pso) {
    return Material();
  }

  // Generate name for debugging
  std::ostringstream oss;
  oss << ShaderRegistry::GetName(shader_id) << "_";
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
  std::string name = oss.str();

  std::wstring wide_name(name.begin(), name.end());
  pso->SetName(wide_name.c_str());

  // Generate sort key
  uint64_t sort_key = GenerateSortKey(rs, pso.Get());

  // Create material
  Material material(name, rs, pso.Get(), sort_key);

  material.SetInstancingSupport(metadata.supports_instancing);

  return material;
}

// ============================================================================
// Frame Lifecycle
// ============================================================================

void MaterialManager::OnFrameEnd() {
  current_frame_++;

  std::unique_lock lock(cache_mutex_);
  if (unified_cache_.size() > MAX_CACHE_SIZE) {
    EvictLRU();
  }
}

void MaterialManager::EvictLRU() {
  if (unified_cache_.empty()) {
    return;
  }

  auto oldest = std::min_element(unified_cache_.begin(), unified_cache_.end(), [](const auto& a, const auto& b) {
    return a.second.last_used_frame < b.second.last_used_frame;
  });

  unified_cache_.erase(oldest);
}
