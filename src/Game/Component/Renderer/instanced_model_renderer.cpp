#include "instanced_model_renderer.h"

#include "Component/render_settings.h"
#include "Framework/Logging/logger.h"
#include "Graphic/Resource/Buffer/instance_buffer_manager.h"
#include "game_context.h"
#include "game_object.h"

InstancedModelRenderer::InstancedModelRenderer(GameObject* owner, const Props& props)
    : Component(owner), model_(props.model), entries_(props.instances) {
  instance_count_ = static_cast<uint32_t>(entries_.size());
  for (uint32_t i = 0; i < instance_count_; ++i) {
    id_to_index_[entries_[i].id] = i;
  }
}

void InstancedModelRenderer::OnInit() {
  if (instance_count_ == 0) return;

  auto& manager = GetOwner()->GetContext()->GetGraphic()->GetInstanceBufferManager();
  buffer_handle_ = manager.Create(instance_count_);

  if (buffer_handle_ == InstanceBufferHandle::Invalid) {
    Logger::LogFormat(LogLevel::Error,
      LogCategory::Game,
      Logger::Here(),
      "[InstancedModelRenderer] Failed to create instance buffer for {} instances",
      instance_count_);
  }
}

void InstancedModelRenderer::OnRender(FramePacket& packet) {
  if (!model_ || instance_count_ == 0 || buffer_handle_ == InstanceBufferHandle::Invalid) return;

  auto* graphic = GetOwner()->GetContext()->GetGraphic();
  auto& manager = graphic->GetInstanceBufferManager();

  if (dirty_) {
    std::vector<GPUInstanceData> gpu_data(instance_count_);
    for (uint32_t i = 0; i < instance_count_; ++i) {
      gpu_data[i].world = entries_[i].props.world;
      gpu_data[i].normal_matrix = entries_[i].props.world.Inverted().Transposed();
      gpu_data[i].color = entries_[i].props.color;
    }
    manager.Update(buffer_handle_, gpu_data.data(), instance_count_);
    dirty_ = false;
  }

  auto& material_mgr = graphic->GetMaterialManager();
  D3D12_GPU_VIRTUAL_ADDRESS buffer_address = manager.GetAddress(buffer_handle_);

  for (const auto& entry : model_->sub_meshes) {
    DrawCommand cmd;
    cmd.mesh = entry.mesh;
    cmd.instance_buffer_address = buffer_address;
    cmd.instance_count = instance_count_;

    auto settings = Rendering::RenderSettings::Opaque();
    cmd.material = material_mgr.GetOrCreateMaterial(Graphics::ModelInstancedShader::ID, settings);

    auto* context = GetOwner()->GetContext();
    Texture* albedo = entry.albedo_texture ? entry.albedo_texture.get() : context->GetAssetManager().GetDefaultWhiteTexture();
    cmd.material_instance.albedo_texture_index = albedo ? albedo->GetBindlessIndex() : 0;
    cmd.material_instance.sampler_index = static_cast<uint32_t>(Rendering::SamplerType::AnisotropicWrap);

    if (entry.normal_texture) {
      cmd.material_instance.normal_texture_index = entry.normal_texture->GetBindlessIndex();
      cmd.material_instance.has_normal_map = true;
    }
    if (entry.metallic_roughness_texture) {
      cmd.material_instance.metallic_roughness_index = entry.metallic_roughness_texture->GetBindlessIndex();
      cmd.material_instance.has_metallic_roughness_map = true;
    }
    if (entry.emissive_texture) {
      cmd.material_instance.emissive_texture_index = entry.emissive_texture->GetBindlessIndex();
      cmd.material_instance.has_emissive_map = true;
    }

    if (entry.surface_material_index < model_->surface_materials.size()) {
      const auto& mat = model_->surface_materials[entry.surface_material_index];
      cmd.color = {mat.base_color_factor.x, mat.base_color_factor.y, mat.base_color_factor.z, mat.base_color_factor.w};
      cmd.material_instance.metallic_factor = mat.metallic_factor;
      cmd.material_instance.roughness_factor = mat.roughness_factor;
      cmd.material_instance.emissive_factor[0] = mat.emissive_factor.x;
      cmd.material_instance.emissive_factor[1] = mat.emissive_factor.y;
      cmd.material_instance.emissive_factor[2] = mat.emissive_factor.z;
    }

    cmd.layer = RenderLayer::Opaque;
    cmd.tags = static_cast<uint32_t>(RenderTag::CastShadow | RenderTag::ReceiveShadow | RenderTag::Lit);
    cmd.depth = 0.0f;

    packet.AddCommand(std::move(cmd));
  }
}

void InstancedModelRenderer::OnDestroy() {
  if (buffer_handle_ == InstanceBufferHandle::Invalid) return;

  auto* graphic = GetOwner()->GetContext()->GetGraphic();
  uint64_t fence_value = graphic->GetCurrentFenceValue();
  graphic->GetInstanceBufferManager().Destroy(buffer_handle_, fence_value);
  buffer_handle_ = InstanceBufferHandle::Invalid;
}

void InstancedModelRenderer::UpdateById(const std::string& id, std::function<InstanceProps(const InstanceProps&)> modifier) {
  auto it = id_to_index_.find(id);
  if (it == id_to_index_.end()) return;

  auto& entry = entries_[it->second];
  entry.props = modifier(entry.props);
  dirty_ = true;
}
