#include "camera_component.h"

#include <DirectXMath.h>

#include "game_object.h"
#include "transform_component.h"

using namespace DirectX;
using Math::Matrix4;
using Math::Vector3;

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
  if (projection_type_ == ProjectionType::Perspective) {
    projection_matrix_ = Matrix4::CreatePerspectiveFOV(fov_, aspect_ratio_, near_plane_, far_plane_);
  } else {
    projection_matrix_ = Matrix4::CreateOrthographic(ortho_width_, ortho_height_, near_plane_, far_plane_);
  }
  is_dirty_ = false;
}

CameraData CameraComponent::GetCameraData() const {
  if (is_dirty_) {
    const_cast<CameraComponent*>(this)->UpdateProjectionMatrix();
  }

  CameraData data;

  auto* transform = owner_->GetComponent<TransformComponent>();
  if (!transform) {
    StoreMatrixToCameraData(data, XMMatrixIdentity(), XMMATRIX(projection_matrix_));
    data.position = Vector3::Zero;
    data.forward = Vector3::Forward;
    data.up = Vector3::Up;
    return data;
  }

  XMMATRIX world = XMMATRIX(transform->GetWorldMatrix());

  XMVECTOR pos = world.r[3];
  data.position = Vector3(pos);

  XMVECTOR forward = XMVector3Normalize(world.r[2]);
  XMVECTOR up = XMVector3Normalize(world.r[1]);
  data.forward = Vector3(forward);
  data.up = Vector3(up);

  XMVECTOR target = XMVectorAdd(pos, forward);

  XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
  StoreMatrixToCameraData(data, view, XMMATRIX(projection_matrix_));

  return data;
}
