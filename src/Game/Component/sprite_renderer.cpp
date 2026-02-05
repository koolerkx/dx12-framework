#include "sprite_renderer.h"

#include "Component/billboard_helper.h"
#include "Component/pivot_type.h"
#include "Game/Asset/asset_manager.h"
#include "Graphic/Pipeline/shader_descriptors.h"
#include "game_context.h"
#include "transform_component.h"

using namespace DirectX;

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
  pivot_.preset = preset;
}

void SpriteRenderer::SetPivot(const Pivot::Config& config) {
  pivot_ = config;
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

DirectX::XMMATRIX SpriteRenderer::CalculateWorldMatrix(TransformComponent* transform, const CameraData& camera) const {
  XMMATRIX world = transform->GetWorldMatrix();

  if (billboard_mode_ == Billboard::Mode::None) {
    return world;
  }

  XMVECTOR scale, rotation, translation;
  XMMatrixDecompose(&scale, &rotation, &translation, world);

  XMFLOAT3 objPos;
  XMStoreFloat3(&objPos, translation);

  XMMATRIX billboardRot;
  if (billboard_mode_ == Billboard::Mode::Cylindrical) {
    billboardRot = Billboard::CreateCylindricalBillboardMatrix(objPos, camera.position);
  } else {
    billboardRot = Billboard::CreateSphericalBillboardMatrix(objPos, camera.position);
  }

  XMMATRIX scaleMat = XMMatrixScalingFromVector(scale);
  XMMATRIX transMat = XMMatrixTranslationFromVector(translation);

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

  XMFLOAT2 normalized_pivot = pivot_.GetNormalized();
  float pivot_offset_x = (normalized_pivot.x - 0.5f);
  float pivot_offset_y = (normalized_pivot.y - 0.5f);

  XMMATRIX pivot_mat = XMMatrixTranslation(pivot_offset_x, pivot_offset_y, 0.0f);
  XMMATRIX size_scale = XMMatrixScaling(size_.x, size_.y, 1.0f);
  XMMATRIX base_world = CalculateWorldMatrix(transform, packet.main_camera);
  XMMATRIX world = size_scale * pivot_mat * base_world;
  XMStoreFloat4x4(&cmd.world_matrix, world);

  XMFLOAT3 worldPos = transform->GetWorldPosition();
  XMFLOAT3 camPos = packet.main_camera.position;
  cmd.depth = XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&worldPos) - XMLoadFloat3(&camPos)));

  cmd.layer = render_layer_;
  cmd.tags = render_tags_;
  packet.AddCommand(std::move(cmd));
}
