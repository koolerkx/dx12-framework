#include "sprite_renderer.h"

#include "Component/billboard_helper.h"
#include "Component/pivot_type.h"
#include "Game/Asset/asset_manager.h"
#include "game_context.h"
#include "transform_component.h"

using namespace DirectX;

void SpriteRenderer::SetPivot(Pivot::Preset preset) {
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
    billboardRot = Billboard::CreateSphericalBillboardMatrix(objPos, camera.position, camera.up);
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

  // Calculate pivot position (Transform Position = Pivot)
  // pivot_offset is the pivot position in content coordinates
  XMFLOAT2 pivot_pos = pivot_.CalculateOffset(size_);
  XMMATRIX pivot_mat = XMMatrixTranslation(-pivot_pos.x, -pivot_pos.y, 0.0f);

  switch (pass_tag_) {
    case RenderPassTag::Ui: {
      UiDrawCommand cmd;
      cmd.mesh = context->GetAssetManager().GetDefaultMesh(DefaultMesh::Quad);
      cmd.material = material_mgr.GetOrCreateMaterial(render_settings_);
      cmd.color = color_;
      cmd.size = size_;
      cmd.layer_id = layer_id_;
      cmd.depth = static_cast<float>(layer_id_);

      // Transform Position = Pivot
      XMMATRIX size_scale = XMMatrixScaling(size_.x, size_.y, 1.0f);
      XMMATRIX world = size_scale * pivot_mat * transform->GetWorldMatrix();
      XMStoreFloat4x4(&cmd.world_matrix, world);

      cmd.material_instance.material = cmd.material;
      cmd.material_instance.albedo_texture_index = texture_->GetBindlessIndex();
      cmd.material_instance.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);

      packet.ui_pass.push_back(cmd);
    } break;

    case RenderPassTag::WorldOpaque: {
      OpaqueDrawCommand cmd;
      cmd.mesh = context->GetAssetManager().GetDefaultMesh(DefaultMesh::Quad);
      cmd.material = material_mgr.GetOrCreateMaterial(render_settings_);
      cmd.color = color_;
      cmd.uv_offset = {0.0f, 0.0f};
      cmd.uv_scale = {1.0f, 1.0f};

      // Transform Position = Pivot
      XMMATRIX size_scale = XMMatrixScaling(size_.x, size_.y, 1.0f);
      XMMATRIX base_world = CalculateWorldMatrix(transform, packet.main_camera);
      XMMATRIX world = size_scale * pivot_mat * base_world;
      XMStoreFloat4x4(&cmd.world_matrix, world);

      // Calculate depth from object position for sorting
      XMFLOAT3 worldPos = transform->GetWorldPosition();
      XMFLOAT3 camPos = packet.main_camera.position;
      cmd.depth = XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&worldPos) - XMLoadFloat3(&camPos)));

      cmd.material_instance.material = cmd.material;
      cmd.material_instance.albedo_texture_index = texture_->GetBindlessIndex();
      cmd.material_instance.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);

      packet.opaque_pass.push_back(cmd);
    } break;

    case RenderPassTag::WorldTransparent: {
      TransparentDrawCommand cmd;
      cmd.mesh = context->GetAssetManager().GetDefaultMesh(DefaultMesh::Quad);
      cmd.material = material_mgr.GetOrCreateMaterial(render_settings_);
      cmd.color = color_;
      cmd.uv_offset = {0.0f, 0.0f};
      cmd.uv_scale = {1.0f, 1.0f};

      // Transform Position = Pivot
      XMMATRIX size_scale = XMMatrixScaling(size_.x, size_.y, 1.0f);
      XMMATRIX base_world = CalculateWorldMatrix(transform, packet.main_camera);
      XMMATRIX world = size_scale * pivot_mat * base_world;
      XMStoreFloat4x4(&cmd.world_matrix, world);

      // Calculate depth from object position for back-to-front sorting
      XMFLOAT3 worldPos = transform->GetWorldPosition();
      XMFLOAT3 camPos = packet.main_camera.position;
      cmd.depth = XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&worldPos) - XMLoadFloat3(&camPos)));

      cmd.material_instance.material = cmd.material;
      cmd.material_instance.albedo_texture_index = texture_->GetBindlessIndex();
      cmd.material_instance.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);

      packet.transparent_pass.push_back(cmd);
    } break;

    default:
      break;
  }
}
