#include "ui_glass_renderer.h"

#include <cstring>

#include "Component/transform_component.h"
#include "Game/Asset/asset_manager.h"
#include "game_context.h"

using Math::Matrix4;
using Math::Vector3;

void UIGlassRenderer::OnRender(FramePacket& packet) {
  auto* context = GetOwner()->GetContext();
  auto* graphic = context->GetGraphic();
  auto& material_mgr = graphic->GetMaterialManager();
  auto* transform = GetOwner()->GetTransform();

  uint32_t blur_srv_index = graphic->GetUIBlurSrvIndex();

  struct GlassParams {
    uint32_t blur_srv_index;
    float distortion_strength;
    float tint_alpha;
    float _pad;
    float tint_r, tint_g, tint_b, tint_a;
  };
  static_assert(sizeof(GlassParams) == 32);

  GlassParams params{
    .blur_srv_index = blur_srv_index,
    .distortion_strength = distortion_strength_,
    .tint_alpha = tint_alpha_,
    ._pad = 0.0f,
    .tint_r = tint_color_.x,
    .tint_g = tint_color_.y,
    .tint_b = tint_color_.z,
    .tint_a = tint_color_.w,
  };

  DrawCommand cmd;
  cmd.mesh = context->GetAssetManager().GetDefaultMesh(DefaultMesh::RoundedRect);
  cmd.material = material_mgr.GetOrCreateMaterial(Graphics::UIGlassShader::ID, {});
  cmd.material_instance.material = cmd.material;
  cmd.color = {1, 1, 1, 1};

  static_assert(sizeof(params) <= sizeof(cmd.custom_data));
  memcpy(cmd.custom_data.data(), &params, sizeof(params));
  cmd.has_custom_data = true;

  cmd.depth = static_cast<float>(layer_id_);

  Vector2 pivot_offset(0.5f - ui_pivot_.x, 0.5f - ui_pivot_.y);
  Matrix4 pivot_mat = Matrix4::CreateTranslation(Vector3(pivot_offset.x, pivot_offset.y, 0.0f));
  Matrix4 size_scale = Matrix4::CreateScale(Vector3(size_.x, size_.y, 1.0f));
  cmd.world_matrix = pivot_mat * size_scale * transform->GetWorldMatrix();

  cmd.layer = RenderLayer::UI;
  cmd.tags = 0;
  cmd.depth_test = false;
  cmd.depth_write = false;
  packet.AddCommand(std::move(cmd));
}
