#pragma once
#include "Component/component.h"
#include "Framework/Math/Math.h"

using Math::Matrix4;
using Math::Quaternion;
using Math::Vector3;

class TransformComponent : public Component<TransformComponent> {
 public:
  struct Props {
    Vector3 position = Vector3::Zero;
    Vector3 scale = Vector3::One;
    Vector3 rotation_degrees = Vector3::Zero;
    Vector3 pivot = Vector3::Zero;   // the scale and rotation center
    Vector3 anchor = Vector3::Zero;  // the position offser
  };

  using Component::Component;

  void Apply(const Props& props);

  void OnStart() override;
  void OnParentChanged() override;

  void SetPosition(const Vector3& pos);
  void SetScale(const Vector3& scale);

  void SetRotation(const Quaternion& quat);
  void SetRotationEuler(float pitch, float yaw, float roll);
  void SetRotationEuler(const Vector3& euler);

  void SetRotationEulerDegree(float pitch, float yaw, float roll);
  void SetRotationEulerDegree(const Vector3& euler);

  void SetPivot(const Vector3& pivot);
  Vector3 GetPivot() const {
    return local_pivot_;
  }

  void SetAnchor(const Vector3& anchor);
  Vector3 GetAnchor() const {
    return local_anchor_;
  }

  Vector3 GetPosition() const {
    return local_pos_;
  }

  Vector3 GetScale() const {
    return local_scale_;
  }

  Quaternion GetRotation() const {
    return local_rot_;
  }

  Vector3 GetRotationDegrees() const {
    return local_euler_degrees_;
  }

  Matrix4 GetWorldMatrix();

  Vector3 GetForward();
  Vector3 GetRight();
  Vector3 GetUp();

  Vector3 GetWorldPosition();
  Vector3 GetWorldScale();

 private:
  void UpdateLocalMatrix();

 private:
  Vector3 local_pos_ = Vector3::Zero;
  Vector3 local_scale_ = Vector3::One;
  Quaternion local_rot_ = Quaternion::Identity;
  Vector3 local_euler_degrees_ = Vector3::Zero;
  Vector3 local_pivot_ = Vector3::Zero;
  Vector3 local_anchor_ = Vector3::Zero;

  Matrix4 local_matrix_;
  Matrix4 world_matrix_;

  bool is_dirty_ = true;

  TransformComponent* parent_transform_ = nullptr;
};
