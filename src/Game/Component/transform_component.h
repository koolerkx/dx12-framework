#pragma once
#include "Component/component.h"
#include "Debug/debug_drawer.h"
#include "Framework/Core/color.h"
#include "Framework/Math/Math.h"
#include "Framework/Serialize/serialize_node.h"

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
    Vector3 anchor = Vector3::Zero;  // the position offset after rotation
  };

  using Component::Component;

  void Apply(const Props& props);

  void OnStart() override;
  void OnParentChanged() override;

  void OnSerialize(framework::SerializeNode& node) const override {
    node.WriteVec3("Position", local_pos_.x, local_pos_.y, local_pos_.z);
    node.WriteVec3("Rotation", local_euler_degrees_.x, local_euler_degrees_.y, local_euler_degrees_.z);
    node.WriteVec3("Scale", local_scale_.x, local_scale_.y, local_scale_.z);
    node.WriteVec3("Pivot", local_pivot_.x, local_pivot_.y, local_pivot_.z);
    node.WriteVec3("Anchor", local_anchor_.x, local_anchor_.y, local_anchor_.z);
  }

  void OnDeserialize(const framework::SerializeNode& node) override {
    Vector3 pos = local_pos_, rot = local_euler_degrees_, scale = local_scale_;
    Vector3 pivot = local_pivot_, anchor = local_anchor_;
    node.ReadVec3("Position", pos.x, pos.y, pos.z);
    node.ReadVec3("Rotation", rot.x, rot.y, rot.z);
    node.ReadVec3("Scale", scale.x, scale.y, scale.z);
    node.ReadVec3("Pivot", pivot.x, pivot.y, pivot.z);
    node.ReadVec3("Anchor", anchor.x, anchor.y, anchor.z);
    SetPosition(pos);
    SetRotationEulerDegree(rot);
    SetScale(scale);
    SetPivot(pivot);
    SetAnchor(anchor);
  }

  void OnDebugDraw(DebugDrawer& drawer) override {
    auto world = GetWorldMatrix();
    float cross_length = 0.3f;

    // Anchor cross: TransformPoint(anchor) = -pivot*S*R + pivot + pos (in parent space)
    // Orbits around pivot under own S/R, independent of anchor value
    Vector3 anchor_world = world.TransformPoint(local_anchor_);

    // World-space directions (rotation only; ignores translation, and ideally ignores scale)
    Vector3 x_dir = world.TransformVector(Vector3(1, 0, 0)).Normalized();
    Vector3 y_dir = world.TransformVector(Vector3(0, 1, 0)).Normalized();
    Vector3 z_dir = world.TransformVector(Vector3(0, 0, 1)).Normalized();

    drawer.DrawLine(anchor_world - x_dir * cross_length, anchor_world + x_dir * cross_length, colors::Blue);
    drawer.DrawLine(anchor_world - y_dir * cross_length, anchor_world + y_dir * cross_length, colors::Red);
    drawer.DrawLine(anchor_world - z_dir * cross_length, anchor_world + z_dir * cross_length, colors::Lime);

    // Pivot sphere: TransformPoint(pivot + anchor) = pivot + pos (in parent space)
    // Independent of own S/R/anchor; parent S/R still applies
    Vector3 pivot_world = world.TransformPoint(local_pivot_ + local_anchor_);
    drawer.DrawWireSphere(pivot_world, 0.1f, colors::Yellow, 8);

    drawer.DrawLine(anchor_world, pivot_world, colors::Gray);
  }

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
