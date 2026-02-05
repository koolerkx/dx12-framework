#include "transform_component.h"

#include <DirectXMath.h>

#include "game_object.h"

using namespace DirectX;

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

void TransformComponent::UpdateLocalMatrix() {
  local_matrix_ = Matrix4::CreateFromTRS(local_pos_, local_rot_, local_scale_);
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
  XMMATRIX world = XMMATRIX(GetWorldMatrix());
  return Vector3(XMVector3Normalize(world.r[2]));
}

Vector3 TransformComponent::GetRight() {
  XMMATRIX world = XMMATRIX(GetWorldMatrix());
  return Vector3(XMVector3Normalize(world.r[0]));
}

Vector3 TransformComponent::GetUp() {
  XMMATRIX world = XMMATRIX(GetWorldMatrix());
  return Vector3(XMVector3Normalize(world.r[1]));
}

Vector3 TransformComponent::GetWorldPosition() const {
  XMMATRIX world = XMMATRIX(world_matrix_);
  return Vector3(world.r[3]);
}

Vector3 TransformComponent::GetWorldScale() const {
  XMMATRIX world = XMMATRIX(world_matrix_);
  return Vector3(
    XMVectorGetX(XMVector3Length(world.r[0])), XMVectorGetX(XMVector3Length(world.r[1])), XMVectorGetX(XMVector3Length(world.r[2])));
}
