#pragma once
#include "Component/component.h"
#include "Framework/Math/Math.h"

using Math::Matrix4;
using Math::Quaternion;
using Math::Vector3;

class TransformComponent : public Component<TransformComponent> {
 public:
  using Component::Component;

  void OnStart() override;
  void OnParentChanged() override;

  void SetPosition(const Vector3& pos);
  void SetScale(const Vector3& scale);

  void SetRotation(const Quaternion& quat);
  void SetRotationEuler(float pitch, float yaw, float roll);
  void SetRotationEuler(const Vector3& euler);

  void SetRotationEulerDegree(float pitch, float yaw, float roll);
  void SetRotationEulerDegree(const Vector3& euler);

  Vector3 GetPosition() const {
    return local_pos_;
  }

  Matrix4 GetWorldMatrix();

  Vector3 GetForward();
  Vector3 GetRight();
  Vector3 GetUp();

  Vector3 GetWorldPosition() const;
  Vector3 GetWorldScale() const;

 private:
  void UpdateLocalMatrix();

 private:
  Vector3 local_pos_ = Vector3::Zero;
  Vector3 local_scale_ = Vector3::One;
  Quaternion local_rot_ = Quaternion::Identity;

  Matrix4 local_matrix_;
  Matrix4 world_matrix_;

  bool is_dirty_ = true;

  TransformComponent* parent_transform_ = nullptr;
};
