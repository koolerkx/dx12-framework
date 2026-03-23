#include "camera_component.h"

#if ENABLE_EDITOR
#include "Framework/Editor/editor_ui.h"
#endif
#include "game_object.h"
#include "transform_component.h"

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
    StoreMatrixToCameraData(data, Matrix4::Identity, projection_matrix_);
    data.position = Vector3::Zero;
    data.forward = Vector3::Forward;
    data.up = Vector3::Up;
    return data;
  }

  Matrix4 world = transform->GetWorldMatrix();

  data.position = world.GetTranslation();
  data.forward = world.GetRow(2).xyz().Normalized();
  data.up = world.GetRow(1).xyz().Normalized();

  Vector3 target = data.position + data.forward;
  Matrix4 view = Matrix4::CreateLookAt(data.position, target, data.up);
  StoreMatrixToCameraData(data, view, projection_matrix_);
  data.exposure = exposure_;

  return data;
}

#if ENABLE_EDITOR
void CameraComponent::OnInspectorGUI() {
  float exposure = exposure_;
  if (editor_ui::DragFloat("Exposure", &exposure, 0.01f, 0.01f, 10.0f)) {
    SetExposure(exposure);
  }
}
#endif
