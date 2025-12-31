#include "camera_component.h"

#include "game_object.h"
#include "transform_component.h"

using namespace DirectX;

void CameraComponent::OnStart() {
  UpdateProjectionMatrix();
}

void CameraComponent::SetPerspective(float fov_radians, float aspect_ratio, float near_plane, float far_plane) {
  projection_type_ = ProjectionType::Perspective;
  fov_ = fov_radians;
  aspect_ratio_ = aspect_ratio;
  near_plane_ = near_plane;
  far_plane_ = far_plane;
  is_dirty_ = true;
}

void CameraComponent::SetOrthographic(float width, float height, float near_plane, float far_plane) {
  projection_type_ = ProjectionType::Orthographic;
  ortho_width_ = width;
  ortho_height_ = height;
  near_plane_ = near_plane;
  far_plane_ = far_plane;
  is_dirty_ = true;
}

void CameraComponent::UpdateProjectionMatrix() {
  XMMATRIX proj;

  if (projection_type_ == ProjectionType::Perspective) {
    proj = XMMatrixPerspectiveFovLH(fov_, aspect_ratio_, near_plane_, far_plane_);
  } else {
    proj = XMMatrixOrthographicLH(ortho_width_, ortho_height_, near_plane_, far_plane_);
  }

  XMStoreFloat4x4(&projection_matrix_, proj);
  is_dirty_ = false;
}

CameraData CameraComponent::GetCameraData() const {
  if (is_dirty_) {
    const_cast<CameraComponent*>(this)->UpdateProjectionMatrix();
  }

  CameraData data;

  // Get transform from owner GameObject
  auto* transform = owner_->GetComponent<TransformComponent>();
  if (!transform) {
    // Fallback to identity
    StoreMatrixToCameraData(data, XMMatrixIdentity(), XMLoadFloat4x4(&projection_matrix_));

    data.position = XMFLOAT3(0, 0, 0);
    data.forward = XMFLOAT3(0, 0, 1);
    data.up = XMFLOAT3(0, 1, 0);
    return data;
  }

  // Build view matrix from transform
  XMMATRIX world = transform->GetWorldMatrix();

  // Extract position from world matrix
  XMVECTOR pos = world.r[3];
  XMStoreFloat3(&data.position, pos);

  // Extract forward and up from world matrix
  XMVECTOR forward = XMVector3Normalize(world.r[2]);  // Z-axis
  XMVECTOR up = XMVector3Normalize(world.r[1]);       // Y-axis
  XMStoreFloat3(&data.forward, forward);
  XMStoreFloat3(&data.up, up);

  // Calculate target point (position + forward)
  XMVECTOR target = XMVectorAdd(pos, forward);

  // Build view matrix (inverse of world transform)
  XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
  StoreMatrixToCameraData(data, view, XMLoadFloat4x4(&projection_matrix_));

  return data;
}
