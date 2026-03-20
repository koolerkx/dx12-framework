#include "instanced_model_renderer.h"

#include "Framework/Render/render_settings.h"
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

  if (model_) {
    auto* graphic = GetOwner()->GetContext()->GetGraphic();
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
  if (!model_ || instance_count_ == 0) return;

  instance_cache_.resize(instance_count_);
  for (uint32_t i = 0; i < instance_count_; ++i) {
    if (!entries_[i].props.visible) {
      instance_cache_[i].world = Math::Matrix4::CreateScale(0.0f);
    } else {
      instance_cache_[i].world = entries_[i].props.world;
    }
    instance_cache_[i].color = entries_[i].props.color;
    instance_cache_[i].uv_offset = {0.0f, 0.0f};
    instance_cache_[i].uv_scale = {1.0f, 1.0f};
    instance_cache_[i].overlay_color = entries_[i].props.overlay_color;
  }

  for (size_t i = 0; i < model_->sub_meshes.size(); ++i) {
    const auto& entry = model_->sub_meshes[i];

    InstancedRenderRequest request;
    request.mesh = entry.mesh_handle;
    request.shader_id = Graphics::PBRShader::ID;
    request.render_settings = Rendering::RenderSettings::Opaque();
    request.material = (i < submesh_material_handles_.size()) ? submesh_material_handles_[i] : MaterialHandle::Invalid();
    if (entry.surface_material_index < model_->surface_materials.size()) {
      const auto& mat = model_->surface_materials[entry.surface_material_index];
      request.color = {mat.base_color_factor.x, mat.base_color_factor.y, mat.base_color_factor.z, mat.base_color_factor.w};
    }
    request.layer = RenderLayer::Opaque;
    request.tags = static_cast<uint32_t>(RenderTag::CastShadow | RenderTag::ReceiveShadow | RenderTag::Lit);
    request.depth = 0.0f;

    packet.DrawInstanced(std::move(request), instance_cache_);
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
  }
  RendererComponent::OnDestroy();
}

void InstancedModelRenderer::UpdateById(const std::string& id, std::function<InstanceProps(const InstanceProps&)> modifier) {
  auto it = id_to_index_.find(id);
  if (it == id_to_index_.end()) return;

  auto& entry = entries_[it->second];
  entry.props = modifier(entry.props);
}
