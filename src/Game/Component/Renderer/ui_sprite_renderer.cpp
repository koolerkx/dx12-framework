#include "ui_sprite_renderer.h"

#include "Game/Asset/asset_manager.h"
#include "Graphic/Pipeline/shader_descriptors.h"
#include "game_context.h"
#include "Component/transform_component.h"

using namespace DirectX;

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
  cmd.mesh = context->GetAssetManager().GetDefaultMesh(DefaultMesh::Quad);
  cmd.material = material_mgr.GetOrCreateMaterial(Graphics::SpriteShader::ID, render_settings_);
  cmd.material_instance.material = cmd.material;
  cmd.material_instance.albedo_texture_index = texture_->GetBindlessIndex();
  cmd.material_instance.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);
  cmd.color = color_;
  cmd.uv_offset = uv_offset_;
  cmd.uv_scale = uv_scale_;

  cmd.depth = static_cast<float>(layer_id_);

  XMMATRIX offset_mat = XMMatrixTranslation(0.5f, 0.5f, 0.0f);
  XMMATRIX size_scale = XMMatrixScaling(size_.x, size_.y, 1.0f);
  XMMATRIX world = offset_mat * size_scale * transform->GetWorldMatrix();
  XMStoreFloat4x4(&cmd.world_matrix, world);

  cmd.layer = RenderLayer::UI;
  cmd.tags = render_tags_;
  packet.AddCommand(std::move(cmd));
}
