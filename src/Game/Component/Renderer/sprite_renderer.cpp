#include "sprite_renderer.h"

#include "Component/pivot_type.h"
#include "Component/transform_component.h"
#include "Framework/Render/shader_ids.h"
#include "Game/Asset/asset_manager.h"
#include "Graphic/Resource/Material/material_descriptor_pool.h"
#include "game_context.h"

void SpriteRenderer::SetRenderLayer(RenderLayer layer) {
  switch (layer) {
    case RenderLayer::Opaque:
      render_settings_ = Rendering::RenderSettings::Opaque();
      break;
    case RenderLayer::Transparent:
      render_settings_ = Rendering::RenderSettings::Transparent();
      break;
    default:
      return;
  }
  render_layer_ = layer;
}

void SpriteRenderer::SetPivot(Pivot::Preset preset) {
  sprite_pivot_ = Pivot::ToSpriteNormalized(preset);
}

void SpriteRenderer::SetPivot(const Vector2& normalized_pivot) {
  sprite_pivot_ = normalized_pivot;
}

SpriteRenderer::EditorData SpriteRenderer::GetEditorData() const {
  return {color_, size_, sprite_pivot_, uv_offset_, uv_scale_, billboard_mode_, render_settings_, render_layer_, render_tags_};
}

void SpriteRenderer::ApplyEditorData(const EditorData& data) {
  SetColor(data.color);
  SetSize(data.size);
  SetPivot(data.pivot);
  SetUVOffset(data.uv_offset);
  SetUVScale(data.uv_scale);
  SetBillboardMode(data.billboard_mode);
  SetRenderLayer(data.render_layer);
  SetBlendMode(data.render_settings.blend_mode);
  SetSampler(data.render_settings.sampler_type);
  SetDepthTest(data.render_settings.depth_test);
  SetDepthWrite(data.render_settings.depth_write);
  SetDoubleSided(data.render_settings.double_sided);
  SetRenderTags(data.render_tags);
}

SpriteSheetAnimator& SpriteRenderer::GetAnimator() {
  if (!animator_) {
    animator_.emplace();
  }
  return *animator_;
}

void SpriteRenderer::OnUpdate(float dt) {
  if (animator_ && animator_->Update(dt)) {
    auto uv = animator_->GetCurrentUV();
    uv_offset_ = uv.uv_offset;
    uv_scale_ = uv.uv_scale;
  }
}

Matrix4 SpriteRenderer::CalculateWorldMatrix(TransformComponent* transform, const CameraData& camera) const {
  Matrix4 world = transform->GetWorldMatrix();

  if (billboard_mode_ == Billboard::Mode::None) {
    return world;
  }

  Vector3 scale = world.GetScale();
  Vector3 translation = world.GetTranslation();

  Vector3 target = camera.position;
  if (billboard_mode_ == Billboard::Mode::Cylindrical) {
    target.y = translation.y;
  }
  Matrix4 billboardRot = Matrix4::FaceTo(translation, target, Vector3::Up);

  Matrix4 scaleMat = Matrix4::CreateScale(scale);
  Matrix4 transMat = Matrix4::CreateTranslation(translation);

  return scaleMat * billboardRot * transMat;
}

void SpriteRenderer::OnRender(FramePacket& packet) {
  if (!texture_.IsValid()) return;

  auto* context = GetOwner()->GetContext();
  auto& pool = context->GetGraphic()->GetMaterialDescriptorPool();
  auto* transform = GetOwner()->GetTransform();

  if (!material_handle_.IsValid() || material_dirty_) {
    MaterialDescriptor desc{};
    desc.albedo_texture_index = texture_.GetBindlessIndex();
    desc.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);
    if (!material_handle_.IsValid()) {
      material_handle_ = pool.Allocate(desc);
    } else {
      pool.Update(material_handle_, desc);
    }
    material_dirty_ = false;
  }

  Vector2 pivot_offset(0.5f - sprite_pivot_.x, sprite_pivot_.y - 0.5f);
  Matrix4 pivot_mat = Matrix4::CreateTranslation(Vector3(pivot_offset.x, pivot_offset.y, 0.0f));
  Matrix4 size_scale = Matrix4::CreateScale(Vector3(size_.x, size_.y, 1.0f));
  Matrix4 base_world = CalculateWorldMatrix(transform, packet.main_camera);

  Vector3 worldPos = transform->GetWorldPosition();
  Vector3 camPos = packet.main_camera.position;

  RenderRequest request;
  request.mesh = context->GetAssetManager().GetDefaultMeshHandle(DefaultMesh::Quad);
  request.shader_id = Shaders::Id::BASIC_3D;
  request.render_settings = render_settings_;
  request.material = material_handle_;
  request.color = color_;
  request.uv_offset = uv_offset_;
  request.uv_scale = uv_scale_;
  request.world_matrix = pivot_mat * size_scale * base_world;
  request.depth = Vector3::DistanceSquared(worldPos, camPos);
  request.layer = render_layer_;
  request.tags = render_tags_;
  packet.Draw(std::move(request));
}

void SpriteRenderer::OnDestroy() {
  if (material_handle_.IsValid()) {
    auto* context = GetOwner()->GetContext();
    if (context && context->GetGraphic()) {
      context->GetGraphic()->GetMaterialDescriptorPool().Free(material_handle_);
    }
    material_handle_ = MaterialHandle::Invalid();
  }
  RendererComponent::OnDestroy();
}
