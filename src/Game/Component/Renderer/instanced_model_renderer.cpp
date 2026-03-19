#include "instanced_model_renderer.h"

#include "Framework/Logging/logger.h"
#include "Framework/Render/render_settings.h"
#include "Graphic/Resource/Buffer/instance_buffer_manager.h"
#include "Graphic/Resource/Material/material_descriptor_pool.h"
#include "game_context.h"
#include "game_object.h"

InstancedModelRenderer::InstancedModelRenderer(GameObject* owner, const Props& props)
    : RendererComponent(owner), model_(props.model), entries_(props.instances) {
  instance_count_ = static_cast<uint32_t>(entries_.size());
  for (uint32_t i = 0; i < instance_count_; ++i) {
    id_to_index_[entries_[i].id] = i;
  }
}

void InstancedModelRenderer::OnInit() {
  RendererComponent::OnInit();
  if (instance_count_ == 0) return;

  auto* graphic = GetOwner()->GetContext()->GetGraphic();
  auto& manager = graphic->GetInstanceBufferManager();
  buffer_handle_ = manager.Create(instance_count_);

  if (buffer_handle_ == InstanceBufferHandle::Invalid) {
    Logger::LogFormat(LogLevel::Error,
      LogCategory::Game,
      Logger::Here(),
      "[InstancedModelRenderer] Failed to create instance buffer for {} instances",
      instance_count_);
  }

  if (model_) {
    auto* context = GetOwner()->GetContext();
    auto& pool = graphic->GetMaterialDescriptorPool();
    submesh_material_handles_.reserve(model_->sub_meshes.size());

    for (const auto& entry : model_->sub_meshes) {
      Texture* albedo = entry.albedo_texture ? entry.albedo_texture.get() : context->GetAssetManager().GetDefaultWhiteTexture();
      MaterialDescriptor desc{};
      desc.albedo_texture_index = albedo ? albedo->GetBindlessIndex() : 0;
      desc.sampler_index = static_cast<uint32_t>(Rendering::SamplerType::AnisotropicWrap);

      if (entry.normal_texture) {
        desc.normal_texture_index = entry.normal_texture->GetBindlessIndex();
        desc.flags |= static_cast<uint32_t>(MaterialFlags::HasNormalMap);
      }
      if (entry.metallic_roughness_texture) {
        desc.metallic_roughness_index = entry.metallic_roughness_texture->GetBindlessIndex();
        desc.flags |= static_cast<uint32_t>(MaterialFlags::HasMetallicRoughnessMap);
      }
      if (entry.emissive_texture) {
        desc.emissive_texture_index = entry.emissive_texture->GetBindlessIndex();
        desc.flags |= static_cast<uint32_t>(MaterialFlags::HasEmissiveMap);
      }

      if (entry.surface_material_index < model_->surface_materials.size()) {
        const auto& mat = model_->surface_materials[entry.surface_material_index];
        desc.metallic_factor = mat.metallic_factor;
        desc.roughness_factor = mat.roughness_factor;
        desc.emissive_factor = Math::Vector3(mat.emissive_factor.x, mat.emissive_factor.y, mat.emissive_factor.z);
      }

      submesh_material_handles_.push_back(pool.Allocate(desc));
    }
  }
}

void InstancedModelRenderer::OnRender(FramePacket& packet) {
  if (!model_ || instance_count_ == 0 || buffer_handle_ == InstanceBufferHandle::Invalid) return;

  auto* graphic = GetOwner()->GetContext()->GetGraphic();
  auto& manager = graphic->GetInstanceBufferManager();

  if (dirty_) {
    std::vector<GPUInstanceData> gpu_data(instance_count_);
    for (uint32_t i = 0; i < instance_count_; ++i) {
      if (!entries_[i].props.visible) {
        gpu_data[i].world = Math::Matrix4::CreateScale(0.0f);
        continue;
      }
      gpu_data[i].world = entries_[i].props.world;
      gpu_data[i].color = entries_[i].props.color;
      gpu_data[i].uv_offset = {0.0f, 0.0f};
      gpu_data[i].uv_scale = {1.0f, 1.0f};
      gpu_data[i].overlay_color = entries_[i].props.overlay_color;
    }
    manager.Update(buffer_handle_, gpu_data.data(), instance_count_);
    dirty_ = false;
  }

  auto& material_mgr = graphic->GetMaterialManager();
  D3D12_GPU_VIRTUAL_ADDRESS buffer_address = manager.GetAddress(buffer_handle_);

  for (size_t i = 0; i < model_->sub_meshes.size(); ++i) {
    const auto& entry = model_->sub_meshes[i];
    DrawCommand cmd;
    cmd.mesh_handle = entry.mesh_handle;
    cmd.instance_buffer_address = buffer_address;
    cmd.instance_count = instance_count_;

    auto settings = Rendering::RenderSettings::Opaque();
    cmd.material = material_mgr.GetOrCreateMaterial(Graphics::PBRShader::ID, settings);

    if (i < submesh_material_handles_.size()) {
      cmd.material_handle = submesh_material_handles_[i];
    }

    if (entry.surface_material_index < model_->surface_materials.size()) {
      const auto& mat = model_->surface_materials[entry.surface_material_index];
      cmd.color = {mat.base_color_factor.x, mat.base_color_factor.y, mat.base_color_factor.z, mat.base_color_factor.w};
    }

    cmd.layer = RenderLayer::Opaque;
    cmd.tags = static_cast<uint32_t>(RenderTag::CastShadow | RenderTag::ReceiveShadow | RenderTag::Lit);
    cmd.depth = 0.0f;

    packet.AddCommand(std::move(cmd));
  }
}

void InstancedModelRenderer::OnDestroy() {
  auto* graphic = GetOwner()->GetContext()->GetGraphic();
  if (graphic) {
    for (auto& handle : submesh_material_handles_) {
      if (handle.IsValid()) {
        graphic->GetMaterialDescriptorPool().Free(handle);
      }
    }
    submesh_material_handles_.clear();

    if (buffer_handle_ != InstanceBufferHandle::Invalid) {
      uint64_t fence_value = graphic->GetCurrentFenceValue();
      graphic->GetInstanceBufferManager().Destroy(buffer_handle_, fence_value);
      buffer_handle_ = InstanceBufferHandle::Invalid;
    }
  }
  RendererComponent::OnDestroy();
}

void InstancedModelRenderer::UpdateById(const std::string& id, std::function<InstanceProps(const InstanceProps&)> modifier) {
  auto it = id_to_index_.find(id);
  if (it == id_to_index_.end()) return;

  auto& entry = entries_[it->second];
  entry.props = modifier(entry.props);
  dirty_ = true;
}
