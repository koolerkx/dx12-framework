#pragma once
#include "Component/component.h"
#include "Framework/Math/Math.h"
#include "Graphic/Frame/camera_data.h"

enum class ProjectionType { Perspective, Orthographic };

class CameraComponent : public Component<CameraComponent> {
 public:
  struct Props {
    float fov = Math::PiOver4;
    float aspect_ratio = 16.0f / 9.0f;
    float near_plane = 0.1f;
    float far_plane = 1000.0f;
  };

  using Component::Component;

  CameraComponent(GameObject* owner, const Props& props) : Component(owner) {
    SetPerspective(props.fov, props.aspect_ratio, props.near_plane, props.far_plane);
  }

  void OnStart() override;

  void SetPerspective(float fov_radians, float aspect_ratio, float near_plane, float far_plane);
  void SetOrthographic(float width, float height, float near_plane, float far_plane);

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

  float GetExposure() const {
    return exposure_;
  }
  void SetExposure(float exposure) {
    exposure_ = exposure;
  }

 private:
  void UpdateProjectionMatrix();

 private:
  ProjectionType projection_type_ = ProjectionType::Perspective;

  float fov_ = Math::PiOver4;
  float aspect_ratio_ = 16.0f / 9.0f;

  float ortho_width_ = 1920.0f;
  float ortho_height_ = 1080.0f;

  float near_plane_ = 0.1f;
  float far_plane_ = 1000.0f;
  float exposure_ = 1.0f;

  Math::Matrix4 projection_matrix_;
  bool is_dirty_ = true;
};
