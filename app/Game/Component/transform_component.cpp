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

  // Hierarchy changed, so world matrix is definitely dirty
  is_dirty_ = true;
}

void TransformComponent::SetPosition(const XMFLOAT3& pos) {
  local_pos_ = pos;
  is_dirty_ = true;
}

void TransformComponent::SetScale(const XMFLOAT3& scale) {
  local_scale_ = scale;
  is_dirty_ = true;
}

void TransformComponent::SetRotation(const XMFLOAT4& quat) {
  local_rot_ = quat;
  is_dirty_ = true;
}

void TransformComponent::SetRotationEuler(float pitch, float yaw, float roll) {
  XMVECTOR quat = XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
  XMStoreFloat4(&local_rot_, quat);
  is_dirty_ = true;
}

void TransformComponent::SetRotationEuler(const XMFLOAT3& euler) {
  SetRotationEuler(euler.x, euler.y, euler.z);
}

void TransformComponent::SetRotationEulerDegree(float pitch, float yaw, float roll) {
  XMVECTOR angles = XMVectorSet(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll), 0.0f);

  XMVECTOR quat = XMQuaternionRotationRollPitchYawFromVector(angles);

  XMStoreFloat4(&local_rot_, quat);
  is_dirty_ = true;
}

void TransformComponent::SetRotationEulerDegree(const XMFLOAT3& euler) {
  SetRotationEulerDegree(euler.x, euler.y, euler.z);
}

void TransformComponent::UpdateLocalMatrix() {
  XMVECTOR T = XMLoadFloat3(&local_pos_);
  XMVECTOR S = XMLoadFloat3(&local_scale_);
  XMVECTOR R = XMLoadFloat4(&local_rot_);
  XMVECTOR Origin = XMVectorZero();

  XMMATRIX mat = XMMatrixAffineTransformation(S, Origin, R, T);
  XMStoreFloat4x4(&local_matrix_, mat);
}

XMMATRIX TransformComponent::GetWorldMatrix() {
  // If dirty, recalculate local matrix first
  if (is_dirty_) {
    UpdateLocalMatrix();
  }

  XMMATRIX local = XMLoadFloat4x4(&local_matrix_);

  // Use the cached parent transform pointer
  if (parent_transform_) {
    XMMATRIX parent_world = parent_transform_->GetWorldMatrix();

    // Multiply: Local * Parent
    XMMATRIX world = XMMatrixMultiply(local, parent_world);
    XMStoreFloat4x4(&world_matrix_, world);

    return world;
  }

  // No parent, world is local
  XMStoreFloat4x4(&world_matrix_, local);
  return local;
}

XMFLOAT3 TransformComponent::GetForward() {
  XMMATRIX world = GetWorldMatrix();
  XMVECTOR forward = XMVector3Normalize(world.r[2]);
  XMFLOAT3 result;
  XMStoreFloat3(&result, forward);
  return result;
}

XMFLOAT3 TransformComponent::GetRight() {
  XMMATRIX world = GetWorldMatrix();
  XMVECTOR right = XMVector3Normalize(world.r[0]);
  XMFLOAT3 result;
  XMStoreFloat3(&result, right);
  return result;
}

XMFLOAT3 TransformComponent::GetUp() {
  XMMATRIX world = GetWorldMatrix();
  XMVECTOR up = XMVector3Normalize(world.r[1]);
  XMFLOAT3 result;
  XMStoreFloat3(&result, up);
  return result;
}
