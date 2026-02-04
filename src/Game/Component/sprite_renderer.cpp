#include "sprite_renderer.h"

#include <cassert>
#include <iostream>

#include "Component/billboard_helper.h"
#include "Component/pivot_type.h"
#include "Game/Asset/asset_manager.h"
#include "Graphic/Pipeline/shader_descriptors.h"
#include "game_context.h"
#include "transform_component.h"

using namespace DirectX;

void SpriteRenderer::SetPivot(Pivot::Preset preset) {
  if (pass_tag_ == RenderPassTag::Ui) {
    assert(true && "ui pass does not support set pivot");
    return;
  }
  pivot_.preset = preset;
}

void SpriteRenderer::SetPivot(const Pivot::Config& config) {
  pivot_ = config;
}

DirectX::XMMATRIX SpriteRenderer::CalculateWorldMatrix(TransformComponent* transform, const CameraData& camera) const {
  // Get the base world matrix from transform
  XMMATRIX world = transform->GetWorldMatrix();

  // If no billboard mode, return the original world matrix
  // Pivot is handled in OnRender
  if (billboard_mode_ == Billboard::Mode::None) {
    return world;
  }

  // Decompose the world matrix into TRS components
  XMVECTOR scale, rotation, translation;
  XMMatrixDecompose(&scale, &rotation, &translation, world);

  // Get object position for billboard calculation
  XMFLOAT3 objPos;
  XMStoreFloat3(&objPos, translation);

  // Calculate billboard rotation based on mode
  // (pivot is at origin, so this works correctly)
  XMMATRIX billboardRot;
  if (billboard_mode_ == Billboard::Mode::Cylindrical) {
    billboardRot = Billboard::CreateCylindricalBillboardMatrix(objPos, camera.position);
  } else {  // Spherical
    billboardRot = Billboard::CreateSphericalBillboardMatrix(objPos, camera.position);
  }

  // Recompose matrix: Scale * BillboardRotation * Translation
  // Note: We preserve position and scale from original transform, but replace rotation with billboard
  XMMATRIX scaleMat = XMMatrixScalingFromVector(scale);
  XMMATRIX transMat = XMMatrixTranslationFromVector(translation);

  return scaleMat * billboardRot * transMat;
}

void SpriteRenderer::OnRender(FramePacket& packet) {
  if (!texture_) return;

  auto* context = GetOwner()->GetContext();
  auto& material_mgr = context->GetGraphic()->GetMaterialManager();
  auto* transform = GetOwner()->GetTransform();

  switch (pass_tag_) {
    case RenderPassTag::Ui: {
      SingleDrawCommand cmd;
      cmd.mesh = context->GetAssetManager().GetDefaultMesh(DefaultMesh::Quad);
      cmd.material = material_mgr.GetOrCreateMaterial(Graphics::SpriteShader::ID, render_settings_);
      cmd.depth = static_cast<float>(layer_id_);

      cmd.material_instance.material = cmd.material;
      cmd.material_instance.albedo_texture_index = texture_->GetBindlessIndex();
      cmd.material_instance.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);

      XMMATRIX offset_mat = XMMatrixTranslation(0.5f, 0.5f, 0.0f);  // Move quad origin to top-left
      XMMATRIX size_scale = XMMatrixScaling(size_.x, size_.y, 1.0f);
      XMMATRIX world = offset_mat * size_scale * transform->GetWorldMatrix();

      XMStoreFloat4x4(&cmd.world_matrix, world);
      cmd.color = color_;
      cmd.uv_offset = uv_offset_;
      cmd.uv_scale = uv_scale_;

      packet.ui_pass.emplace_back(cmd);
    } break;

    case RenderPassTag::WorldOpaque:
    case RenderPassTag::WorldTransparent: {
      // World pass: use pivot system
      // Calculate pivot offset based on preset
      XMFLOAT2 normalized_pivot = pivot_.GetNormalized();  // (0.5, 0.5) for Center, (0.5, 1.0) for Bottom
      // Convert to quad-local offset: we want to move the quad so the pivot point is at origin
      // Quad vertex range is [-0.5, 0.5], so offset = normalized_pivot - 0.5
      float pivot_offset_x = (normalized_pivot.x - 0.5f);
      float pivot_offset_y = (normalized_pivot.y - 0.5f);

      XMMATRIX pivot_mat = XMMatrixTranslation(pivot_offset_x, pivot_offset_y, 0.0f);
      XMMATRIX size_scale = XMMatrixScaling(size_.x, size_.y, 1.0f);
      XMMATRIX base_world = CalculateWorldMatrix(transform, packet.main_camera);
      XMMATRIX world = size_scale * pivot_mat * base_world;

      SingleDrawCommand cmd;
      cmd.mesh = context->GetAssetManager().GetDefaultMesh(DefaultMesh::Quad);
      cmd.material = material_mgr.GetOrCreateMaterial(Graphics::SpriteShader::ID, render_settings_);

      // Calculate depth from object position for sorting
      XMFLOAT3 worldPos = transform->GetWorldPosition();
      XMFLOAT3 camPos = packet.main_camera.position;
      cmd.depth = XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&worldPos) - XMLoadFloat3(&camPos)));

      cmd.material_instance.material = cmd.material;
      cmd.material_instance.albedo_texture_index = texture_->GetBindlessIndex();
      cmd.material_instance.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);

      XMStoreFloat4x4(&cmd.world_matrix, world);
      cmd.color = color_;
      cmd.uv_offset = uv_offset_;
      cmd.uv_scale = uv_scale_;

      if (pass_tag_ == RenderPassTag::WorldOpaque) {
        packet.opaque_pass.emplace_back(cmd);
      } else {
        packet.transparent_pass.emplace_back(cmd);
      }
    } break;

    default:
      break;
  }
}
