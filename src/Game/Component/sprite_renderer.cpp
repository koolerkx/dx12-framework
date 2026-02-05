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

  DrawCommand cmd;
  cmd.mesh = context->GetAssetManager().GetDefaultMesh(DefaultMesh::Quad);
  cmd.material = material_mgr.GetOrCreateMaterial(Graphics::SpriteShader::ID, render_settings_);
  cmd.material_instance.material = cmd.material;
  cmd.material_instance.albedo_texture_index = texture_->GetBindlessIndex();
  cmd.material_instance.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);
  cmd.color = color_;
  cmd.uv_offset = uv_offset_;
  cmd.uv_scale = uv_scale_;

  // Sync render_layer_ with pass_tag_ for backward compatibility
  switch (pass_tag_) {
    case RenderPassTag::Ui:
      render_layer_ = RenderLayer::UI;
      cmd.depth = static_cast<float>(layer_id_);

      {
        XMMATRIX offset_mat = XMMatrixTranslation(0.5f, 0.5f, 0.0f);
        XMMATRIX size_scale = XMMatrixScaling(size_.x, size_.y, 1.0f);
        XMMATRIX world = offset_mat * size_scale * transform->GetWorldMatrix();
        XMStoreFloat4x4(&cmd.world_matrix, world);
      }
      break;

    case RenderPassTag::WorldOpaque:
      render_layer_ = RenderLayer::Opaque;
      {
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
      }
      break;

    case RenderPassTag::WorldTransparent:
      render_layer_ = RenderLayer::Transparent;
      {
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
      }
      break;

    default:
      break;
  }

  // Use new unified command system
  packet.AddCommand(render_layer_, std::move(cmd), render_tags_);

  // DEPRECATED: Keep old system for backward compatibility
  switch (pass_tag_) {
    case RenderPassTag::Ui: {
      SingleDrawCommand old_cmd;
      old_cmd.mesh = cmd.mesh;
      old_cmd.material = cmd.material;
      old_cmd.material_instance = cmd.material_instance;
      old_cmd.world_matrix = cmd.world_matrix;
      old_cmd.color = cmd.color;
      old_cmd.depth = cmd.depth;
      old_cmd.uv_offset = cmd.uv_offset;
      old_cmd.uv_scale = cmd.uv_scale;
      packet.ui_pass.emplace_back(old_cmd);
    } break;

    case RenderPassTag::WorldOpaque: {
      SingleDrawCommand old_cmd;
      old_cmd.mesh = cmd.mesh;
      old_cmd.material = cmd.material;
      old_cmd.material_instance = cmd.material_instance;
      old_cmd.world_matrix = cmd.world_matrix;
      old_cmd.color = cmd.color;
      old_cmd.depth = cmd.depth;
      old_cmd.uv_offset = cmd.uv_offset;
      old_cmd.uv_scale = cmd.uv_scale;
      packet.opaque_pass.emplace_back(old_cmd);
    } break;

    case RenderPassTag::WorldTransparent: {
      SingleDrawCommand old_cmd;
      old_cmd.mesh = cmd.mesh;
      old_cmd.material = cmd.material;
      old_cmd.material_instance = cmd.material_instance;
      old_cmd.world_matrix = cmd.world_matrix;
      old_cmd.color = cmd.color;
      old_cmd.depth = cmd.depth;
      old_cmd.uv_offset = cmd.uv_offset;
      old_cmd.uv_scale = cmd.uv_scale;
      packet.transparent_pass.emplace_back(old_cmd);
    } break;

    default:
      break;
  }
}
