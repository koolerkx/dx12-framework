#include "instanced_mesh_renderer.h"

#include "Framework/Render/render_settings.h"
#include "Framework/Render/render_handles.h"
#include "Framework/Shader/default_shaders.h"
#include "game_context.h"
#include "game_object.h"

InstancedMeshRenderer::InstancedMeshRenderer(GameObject* owner, const Props& props)
    : RendererComponent(owner), mesh_type_(props.mesh_type), entries_(props.instances) {
  instance_count_ = static_cast<uint32_t>(entries_.size());
}

void InstancedMeshRenderer::OnInit() {
  RendererComponent::OnInit();
}

void InstancedMeshRenderer::OnRender(FramePacket& packet) {
  if (instance_count_ == 0) return;

  auto* context = GetOwner()->GetContext();
  auto* rs = context->GetRenderService();

  MeshHandle mesh_handle = context->GetAssetManager().GetDefaultMeshHandle(mesh_type_);
  if (!mesh_handle.IsValid()) return;

  if (!material_handle_.IsValid()) {
    TextureHandle albedo = context->GetAssetManager().GetDefaultWhiteTexture();
    MaterialDescriptor desc{};
    desc.albedo_texture_index = albedo.IsValid() ? albedo.GetBindlessIndex() : 0;
    desc.sampler_index = static_cast<uint32_t>(Rendering::SamplerType::AnisotropicWrap);
    material_handle_ = rs->AllocateMaterial(desc);
  }

  instance_cache_.resize(instance_count_);
  for (uint32_t i = 0; i < instance_count_; ++i) {
    instance_cache_[i].world = entries_[i].world;
    instance_cache_[i].color = entries_[i].color;
    instance_cache_[i].uv_offset = {0.0f, 0.0f};
    instance_cache_[i].uv_scale = {1.0f, 1.0f};
    instance_cache_[i].overlay_color = entries_[i].overlay_color;
  }

  InstancedRenderRequest request;
  request.SetShader<Shaders::PBR>();
  request.render_settings = Rendering::RenderSettings::Opaque();
  request.mesh = mesh_handle;
  request.material = material_handle_;
  request.layer = RenderLayer::Opaque;
  request.tags = static_cast<uint32_t>(RenderTag::CastShadow | RenderTag::ReceiveShadow | RenderTag::Lit);
  request.depth = 0.0f;

  packet.DrawInstanced(std::move(request), instance_cache_);
}

void InstancedMeshRenderer::OnDestroy() {
  if (material_handle_.IsValid()) {
    auto* rs = GetOwner()->GetContext()->GetRenderService();
    if (rs) {
      rs->FreeMaterial(material_handle_);
    }
    material_handle_ = MaterialHandle::Invalid();
  }
  RendererComponent::OnDestroy();
}
