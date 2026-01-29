#pragma once
#include <DirectXMath.h>

#include "Component/component.h"
#include "Graphic/Frame/camera_data.h"

enum class ProjectionType { Perspective, Orthographic };

class CameraComponent : public Component<CameraComponent> {
 public:
  using Component::Component;

  void OnStart() override;

  // Camera settings
  void SetPerspective(float fov_radians, float aspect_ratio, float near_plane, float far_plane);
  void SetOrthographic(float width, float height, float near_plane, float far_plane);

  // Extract camera data for rendering
  CameraData GetCameraData() const;

  ProjectionType GetProjectionType() const {
    return projection_type_;
  }

  float GetFOV() const {
    return fov_;
  }
  float GetNearPlane() const {
    return near_plane_;
  }
  float GetFarPlane() const {
    return far_plane_;
  }

 private:
  void UpdateProjectionMatrix();

 private:
  ProjectionType projection_type_ = ProjectionType::Perspective;

  // Perspective settings
  float fov_ = DirectX::XM_PIDIV4;  // 45 degrees
  float aspect_ratio_ = 16.0f / 9.0f;

  // Orthographic settings
  float ortho_width_ = 1920.0f;
  float ortho_height_ = 1080.0f;

  // Common settings
  float near_plane_ = 0.1f;
  float far_plane_ = 1000.0f;

  DirectX::XMFLOAT4X4 projection_matrix_;
  bool is_dirty_ = true;
};
