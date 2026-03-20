#include "ui_glass_renderer.h"

#include <cstdio>
#include <cstring>

#include "Component/transform_component.h"
#include "Game/Asset/asset_manager.h"
#include "Graphic/Resource/mesh_factory.h"
#include "game_context.h"

using Math::Matrix4;
using Math::Vector3;

void UIGlassRenderer::OnRender(FramePacket& packet) {
  auto* context = GetOwner()->GetContext();
  auto* graphic = context->GetGraphic();
  auto* transform = GetOwner()->GetTransform();

  uint32_t blur_srv_index = graphic->GetUIBlurSrvIndex();

  struct GlassParams {
    uint32_t blur_srv_index;    // SRV index into bindless texture array for blurred background
    float distortion_strength;  // UV offset magnitude for refraction (higher = more warping)
    float tint_alpha;           // blend factor between blurred background and tint color
    float chromatic_strength;   // R/G/B channel separation along distortion direction
    float tint_r, tint_g, tint_b, tint_a;
    float fresnel_power;       // exponent controlling rim light falloff (higher = thinner rim)
    float fresnel_intensity;   // brightness of the rim/edge glow
    float specular_intensity;  // peak brightness of the fake specular highlight blob
    float specular_sharpness;  // gaussian falloff rate (higher = smaller, sharper highlight)
    float specular_offset_x;   // highlight center offset from panel center in UV space
    float specular_offset_y;
    float edge_shadow_strength;  // darkening at panel edges via smoothstep vignette
    float panel_aspect;          // width / height for uniform corner rounding
    float darken;                // 0.0 = no darkening, 1.0 = fully black
    float _pad[3] = {};
  };
  static_assert(sizeof(GlassParams) == 80);

  GlassParams params{
    .blur_srv_index = blur_srv_index,
    .distortion_strength = distortion_strength_,
    .tint_alpha = tint_alpha_,
    .chromatic_strength = chromatic_strength_,
    .tint_r = tint_color_.x,
    .tint_g = tint_color_.y,
    .tint_b = tint_color_.z,
    .tint_a = tint_color_.w,
    .fresnel_power = fresnel_power_,
    .fresnel_intensity = fresnel_intensity_,
    .specular_intensity = specular_intensity_,
    .specular_sharpness = specular_sharpness_,
    .specular_offset_x = specular_offset_.x,
    .specular_offset_y = specular_offset_.y,
    .edge_shadow_strength = edge_shadow_strength_,
    .panel_aspect = size_.x / size_.y,
    .darken = darken_,
  };

  float aspect = size_.x / size_.y;
  float min_dim = (std::min)(size_.x, size_.y);
  float mesh_radius = (min_dim > 0.0f) ? corner_radius_px_ / min_dim : 0.1f;
  char mesh_key[64];
  std::snprintf(mesh_key, sizeof(mesh_key), "glass_rounded_rect:%.3f:%.4f", aspect, mesh_radius);

  auto mesh_data = MeshFactory::CreateRoundedRectData(mesh_radius, 8, aspect);
  MeshHandle mesh_handle = context->GetAssetManager().GetOrCreateMeshHandle(mesh_key, mesh_data);
  if (!mesh_handle.IsValid()) {
    mesh_handle = context->GetAssetManager().GetDefaultMeshHandle(DefaultMesh::RoundedRect);
  }

  Vector2 pivot_offset(0.5f - ui_pivot_.x, 0.5f - ui_pivot_.y);
  Matrix4 pivot_mat = Matrix4::CreateTranslation(Vector3(pivot_offset.x, pivot_offset.y, 0.0f));
  Matrix4 size_scale = Matrix4::CreateScale(Vector3(size_.x, size_.y, 1.0f));

  RenderRequest request;
  request.mesh = mesh_handle;
  request.shader_id = Graphics::UIGlassShader::ID;
  request.render_settings = Graphics::UIGlassShader::DefaultRenderSettings();
  request.color = {1, 1, 1, 1};
  request.world_matrix = pivot_mat * size_scale * transform->GetWorldMatrix();
  request.depth = static_cast<float>(layer_id_);
  request.layer = RenderLayer::UI;
  request.tags = 0;

  static_assert(sizeof(params) <= sizeof(request.custom_data.data));
  memcpy(request.custom_data.data.data(), &params, sizeof(params));
  request.custom_data.active = true;

  packet.Draw(std::move(request));
}
