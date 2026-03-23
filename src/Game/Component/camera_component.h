#pragma once
#include "Component/component.h"
#include "Framework/Math/Math.h"
#include "Framework/Serialize/serialize_node.h"
#include "Framework/Render/camera_data.h"

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

  void OnSerialize(framework::SerializeNode& node) const override {
    node.Write("ProjectionType", projection_type_ == ProjectionType::Perspective ? "Perspective" : "Orthographic");
    node.Write("FOV", fov_);
    node.Write("AspectRatio", aspect_ratio_);
    node.Write("OrthoWidth", ortho_width_);
    node.Write("OrthoHeight", ortho_height_);

    node.Write("NearPlane", near_plane_);
    node.Write("FarPlane", far_plane_);

    node.Write("Exposure", exposure_);
    node.Write("Priority", priority_);
  }

  void OnDeserialize(const framework::SerializeNode& node) override {
    auto proj_str = node.ReadString("ProjectionType", "Perspective");
    float fov = node.ReadFloat("FOV", fov_);
    float aspect = node.ReadFloat("AspectRatio", aspect_ratio_);
    float near_p = node.ReadFloat("NearPlane", near_plane_);
    float far_p = node.ReadFloat("FarPlane", far_plane_);

    if (proj_str == "Orthographic") {
      float w = node.ReadFloat("OrthoWidth", ortho_width_);
      float h = node.ReadFloat("OrthoHeight", ortho_height_);
      SetOrthographic(w, h, near_p, far_p);
    } else {
      SetPerspective(fov, aspect, near_p, far_p);
    }

    SetExposure(node.ReadFloat("Exposure", exposure_));
    priority_ = node.ReadInt("Priority", 0);
  }

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

  int GetPriority() const {
    return priority_;
  }
  void SetPriority(int priority) {
    priority_ = priority;
  }

#if ENABLE_EDITOR
  void OnInspectorGUI() override;
#endif

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
  int priority_ = 0;

  Math::Matrix4 projection_matrix_;
  bool is_dirty_ = true;
};
