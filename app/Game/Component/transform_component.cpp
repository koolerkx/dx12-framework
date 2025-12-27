#include "transform_component.h"

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
