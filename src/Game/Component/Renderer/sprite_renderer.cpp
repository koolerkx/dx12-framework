#include "sprite_renderer.h"

#include "Component/pivot_type.h"
#include "Component/transform_component.h"
#include "Game/Asset/asset_manager.h"
#include "Graphic/Pipeline/shader_descriptors.h"
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
  return {color_, size_, sprite_pivot_, uv_offset_, uv_scale_,
          billboard_mode_, render_settings_, render_layer_, render_tags_};
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
  if (!texture_) return;

  auto* context = GetOwner()->GetContext();
  auto& material_mgr = context->GetGraphic()->GetMaterialManager();
  auto* transform = GetOwner()->GetTransform();

  DrawCommand cmd;
  cmd.mesh = context->GetAssetManager().GetDefaultMesh(DefaultMesh::Quad);
  cmd.material = material_mgr.GetOrCreateMaterial(Graphics::SpriteShader::ID, render_settings_);
  cmd.material_instance.material = cmd.material;
  cmd.material_instance.albedo_texture_index = texture_->GetBindlessIndex();
  cmd.material_instance.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);
  cmd.color = color_;
  cmd.uv_offset = uv_offset_;
  cmd.uv_scale = uv_scale_;

  Vector2 pivot_offset(0.5f - sprite_pivot_.x, sprite_pivot_.y - 0.5f);
  Matrix4 pivot_mat = Matrix4::CreateTranslation(Vector3(pivot_offset.x, pivot_offset.y, 0.0f));
  Matrix4 size_scale = Matrix4::CreateScale(Vector3(size_.x, size_.y, 1.0f));
  Matrix4 base_world = CalculateWorldMatrix(transform, packet.main_camera);
  cmd.world_matrix = pivot_mat * size_scale * base_world;

  Vector3 worldPos = transform->GetWorldPosition();
  Vector3 camPos = packet.main_camera.position;
  cmd.depth = Vector3::DistanceSquared(worldPos, camPos);

  cmd.layer = render_layer_;
  cmd.tags = render_tags_;
  packet.AddCommand(std::move(cmd));
}
