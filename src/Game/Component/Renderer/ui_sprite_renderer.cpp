#include "ui_sprite_renderer.h"

#include "Component/pivot_type.h"
#include "Component/transform_component.h"
#include "Framework/Render/shader_ids.h"
#include "Game/Asset/asset_manager.h"
#include "Graphic/Resource/Material/material_descriptor_pool.h"
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
  return {color_, size_, ui_pivot_, uv_offset_, uv_scale_, layer_id_, render_settings_, render_tags_};
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

  Vector2 pivot_offset(0.5f - ui_pivot_.x, 0.5f - ui_pivot_.y);
  Matrix4 pivot_mat = Matrix4::CreateTranslation(Vector3(pivot_offset.x, pivot_offset.y, 0.0f));
  Matrix4 size_scale = Matrix4::CreateScale(Vector3(size_.x, size_.y, 1.0f));

  RenderRequest request;
  request.mesh = context->GetAssetManager().GetDefaultMeshHandle(DefaultMesh::Rect);
  request.shader_id = Shaders::Id::SPRITE;
  request.render_settings = render_settings_;
  request.material = material_handle_;
  request.color = color_;
  request.uv_offset = uv_offset_;
  request.uv_scale = uv_scale_;
  request.world_matrix = pivot_mat * size_scale * transform->GetWorldMatrix();
  request.depth = static_cast<float>(layer_id_);
  request.layer = RenderLayer::UI;
  request.tags = render_tags_;
  packet.Draw(std::move(request));
}

void UISpriteRenderer::OnDestroy() {
  if (material_handle_.IsValid()) {
    auto* context = GetOwner()->GetContext();
    if (context && context->GetGraphic()) {
      context->GetGraphic()->GetMaterialDescriptorPool().Free(material_handle_);
    }
    material_handle_ = MaterialHandle::Invalid();
  }
  RendererComponent::OnDestroy();
}
