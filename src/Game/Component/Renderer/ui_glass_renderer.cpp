#include "ui_glass_renderer.h"

#include <cstdio>
#include <cstring>

#include "Component/transform_component.h"
#include "Framework/Asset/asset_manager.h"
#include "Framework/Render/mesh_data_factory.h"
#include "Framework/Shader/default_shaders.h"
#include "Framework/Shader/shader_descriptor.h"
#include "Framework/Shader/shader_registration.h"
#include "Framework/Shader/vertex_shaders.h"
#include "GlassCB.generated.h"
#include "game_context.h"

using Math::Matrix4;
using Math::Vector3;

void UIGlassRenderer::OnInit() {
  RendererComponent::OnInit();

  if (auto* reg = GetOwner()->GetContext()->GetShaderRegistration()) {
    ShaderDescriptor desc{
      .id = HashShaderName("UIGlass"),
      .name = "UIGlass",
      .vs_path = VS::Sprite::PATH,
      .ps_path = L"Content/shaders/ui_glass.ps.cso",
      .vertex_format = VS::Sprite::VERTEX_FORMAT,
      .default_settings =
        {
          .blend_mode = Rendering::BlendMode::AlphaBlend,
          .render_target_format = Rendering::RenderTargetFormat::SDR,
        },
    };
    reg->RegisterShader(desc);
    shader_id_ = desc.id;
  }
}

void UIGlassRenderer::OnRender(FramePacket& packet) {
  auto* context = GetOwner()->GetContext();
  auto* transform = GetOwner()->GetTransform();

  uint32_t blur_srv_index = context->GetRenderService()->GetUIBlurSrvIndex();

  GlassCB params{};
  params.blurSrvIndex = blur_srv_index;
  params.distortionStrength = distortion_strength_;
  params.tintAlpha = tint_alpha_;
  params.chromaticStrength = chromatic_strength_;
  params.tintColor = {tint_color_.x, tint_color_.y, tint_color_.z, tint_color_.w};
  params.fresnelPower = fresnel_power_;
  params.fresnelIntensity = fresnel_intensity_;
  params.specularIntensity = specular_intensity_;
  params.specularSharpness = specular_sharpness_;
  params.specularOffsetX = specular_offset_.x;
  params.specularOffsetY = specular_offset_.y;
  params.edgeShadowStrength = edge_shadow_strength_;
  params.panelAspect = size_.x / size_.y;
  params.darken = darken_;

  float aspect = size_.x / size_.y;
  float min_dim = (std::min)(size_.x, size_.y);
  float mesh_radius = (min_dim > 0.0f) ? corner_radius_px_ / min_dim : 0.1f;
  char mesh_key[64];
  std::snprintf(mesh_key, sizeof(mesh_key), "glass_rounded_rect:%.3f:%.4f", aspect, mesh_radius);

  auto mesh_data = MeshDataFactory::CreateRoundedRectData(mesh_radius, 8, aspect);
  MeshHandle mesh_handle = context->GetAssetManager().GetOrCreateMeshHandle(mesh_key, mesh_data);
  if (!mesh_handle.IsValid()) {
    mesh_handle = context->GetAssetManager().GetDefaultMeshHandle(DefaultMesh::RoundedRect);
  }

  Vector2 pivot_offset(0.5f - ui_pivot_.x, 0.5f - ui_pivot_.y);
  Matrix4 pivot_mat = Matrix4::CreateTranslation(Vector3(pivot_offset.x, pivot_offset.y, 0.0f));
  Matrix4 size_scale = Matrix4::CreateScale(Vector3(size_.x, size_.y, 1.0f));

  RenderRequest request;
  request.SetShader(shader_id_);
  request.render_settings = {
    .blend_mode = Rendering::BlendMode::AlphaBlend,
    .render_target_format = Rendering::RenderTargetFormat::SDR,
  };
  request.mesh = mesh_handle;
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
