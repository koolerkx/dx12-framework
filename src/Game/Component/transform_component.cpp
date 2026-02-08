#include "transform_component.h"

#include "game_object.h"

void TransformComponent::Apply(const Props& props) {
  if (props.position != Vector3::Zero) SetPosition(props.position);
  if (props.scale != Vector3::One) SetScale(props.scale);
  if (props.rotation_degrees != Vector3::Zero) SetRotationEulerDegree(props.rotation_degrees);
  if (props.pivot != Vector3::Zero) SetPivot(props.pivot);
}

void TransformComponent::OnStart() {
  UpdateLocalMatrix();
  OnParentChanged();
}

void TransformComponent::OnParentChanged() {
  GameObject* parent_go = owner_->GetParent();
  if (parent_go) {
    parent_transform_ = parent_go->GetComponent<TransformComponent>();
  } else {
    parent_transform_ = nullptr;
  }

  is_dirty_ = true;
}

void TransformComponent::SetPosition(const Vector3& pos) {
  local_pos_ = pos;
  is_dirty_ = true;
}

void TransformComponent::SetScale(const Vector3& scale) {
  local_scale_ = scale;
  is_dirty_ = true;
}

void TransformComponent::SetRotation(const Quaternion& quat) {
  local_rot_ = quat;
  is_dirty_ = true;
}

void TransformComponent::SetRotationEuler(float pitch, float yaw, float roll) {
  local_rot_ = Quaternion::CreateFromEulerAngles(pitch, yaw, roll);
  is_dirty_ = true;
}

void TransformComponent::SetRotationEuler(const Vector3& euler) {
  SetRotationEuler(euler.x, euler.y, euler.z);
}

void TransformComponent::SetRotationEulerDegree(float pitch, float yaw, float roll) {
  local_rot_ = Quaternion::CreateFromEulerAngles(Math::ToRadians(pitch), Math::ToRadians(yaw), Math::ToRadians(roll));
  is_dirty_ = true;
}

void TransformComponent::SetRotationEulerDegree(const Vector3& euler) {
  SetRotationEulerDegree(euler.x, euler.y, euler.z);
}

void TransformComponent::SetPivot(const Vector3& pivot) {
  local_pivot_ = pivot;
  is_dirty_ = true;
}

void TransformComponent::UpdateLocalMatrix() {
  if (local_pivot_ == Vector3::Zero) {
    local_matrix_ = Matrix4::CreateFromTRS(local_pos_, local_rot_, local_scale_);
    return;
  }

  // M = T(-pivot) * S * R * T(pivot + pos)
  Matrix4 neg_pivot = Matrix4::CreateTranslation(-local_pivot_);
  Matrix4 scale = Matrix4::CreateScale(local_scale_);
  Matrix4 rotation = Matrix4::CreateFromQuaternion(local_rot_);
  Matrix4 pos = Matrix4::CreateTranslation(local_pivot_ + local_pos_);

  local_matrix_ = neg_pivot * scale * rotation * pos;
}

Matrix4 TransformComponent::GetWorldMatrix() {
  if (is_dirty_) {
    UpdateLocalMatrix();
  }

  Matrix4 local = local_matrix_;

  if (parent_transform_) {
    Matrix4 parent_world = parent_transform_->GetWorldMatrix();
    world_matrix_ = local * parent_world;
    return world_matrix_;
  }

  world_matrix_ = local;
  return local;
}

Vector3 TransformComponent::GetForward() {
  return GetWorldMatrix().GetRow(2).xyz().Normalized();
}

Vector3 TransformComponent::GetRight() {
  return GetWorldMatrix().GetRow(0).xyz().Normalized();
}

Vector3 TransformComponent::GetUp() {
  return GetWorldMatrix().GetRow(1).xyz().Normalized();
}

Vector3 TransformComponent::GetWorldPosition() const {
  return world_matrix_.GetTranslation();
}

Vector3 TransformComponent::GetWorldScale() const {
  return world_matrix_.GetScale();
}
