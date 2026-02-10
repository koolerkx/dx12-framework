#include "ui_sprite_renderer.h"

#include "Component/pivot_type.h"
#include "Component/transform_component.h"
#include "Game/Asset/asset_manager.h"
#include "Graphic/Pipeline/shader_descriptors.h"
#include "game_context.h"

using Math::Matrix4;
using Math::Vector3;

void UISpriteRenderer::SetPivot(Pivot::Preset preset) {
  ui_pivot_ = Pivot::ToUISpriteNormalized(preset);
}

void UISpriteRenderer::SetPivot(const Vector2& normalized_pivot) {
  ui_pivot_ = normalized_pivot;
}

UISpriteRenderer::EditorData UISpriteRenderer::GetEditorData() const {
  return {color_, size_, ui_pivot_, uv_offset_, uv_scale_,
          layer_id_, render_settings_, render_tags_};
}

void UISpriteRenderer::ApplyEditorData(const EditorData& data) {
  SetColor(data.color);
  SetSize(data.size);
  SetPivot(data.pivot);
  SetUVOffset(data.uv_offset);
  SetUVScale(data.uv_scale);
  SetLayerId(data.layer_id);
  SetBlendMode(data.render_settings.blend_mode);
  SetSampler(data.render_settings.sampler_type);
  SetRenderTags(data.render_tags);
}

SpriteSheetAnimator& UISpriteRenderer::GetAnimator() {
  if (!animator_) {
    animator_.emplace();
  }
  return *animator_;
}

void UISpriteRenderer::OnUpdate(float dt) {
  if (animator_ && animator_->Update(dt)) {
    auto uv = animator_->GetCurrentUV();
    uv_offset_ = uv.uv_offset;
    uv_scale_ = uv.uv_scale;
  }
}

void UISpriteRenderer::OnRender(FramePacket& packet) {
  if (!texture_) return;

  auto* context = GetOwner()->GetContext();
  auto& material_mgr = context->GetGraphic()->GetMaterialManager();
  auto* transform = GetOwner()->GetTransform();

  DrawCommand cmd;
  cmd.mesh = context->GetAssetManager().GetDefaultMesh(DefaultMesh::Rect);
  cmd.material = material_mgr.GetOrCreateMaterial(Graphics::SpriteShader::ID, render_settings_);
  cmd.material_instance.material = cmd.material;
  cmd.material_instance.albedo_texture_index = texture_->GetBindlessIndex();
  cmd.material_instance.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);
  cmd.color = color_;
  cmd.uv_offset = uv_offset_;
  cmd.uv_scale = uv_scale_;

  cmd.depth = static_cast<float>(layer_id_);

  Vector2 pivot_offset(0.5f - ui_pivot_.x, 0.5f - ui_pivot_.y);
  Matrix4 pivot_mat = Matrix4::CreateTranslation(Vector3(pivot_offset.x, pivot_offset.y, 0.0f));
  Matrix4 size_scale = Matrix4::CreateScale(Vector3(size_.x, size_.y, 1.0f));
  cmd.world_matrix = pivot_mat * size_scale * transform->GetWorldMatrix();

  cmd.layer = RenderLayer::UI;
  cmd.tags = render_tags_;
  packet.AddCommand(std::move(cmd));
}
