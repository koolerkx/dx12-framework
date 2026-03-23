#include "transform_component.h"

#if ENABLE_EDITOR
#include "Framework/Editor/editor_ui.h"
#endif
#include "game_object.h"

void TransformComponent::Apply(const Props& props) {
  if (props.position != Vector3::Zero) SetPosition(props.position);
  if (props.scale != Vector3::One) SetScale(props.scale);
  if (props.rotation_degrees != Vector3::Zero) SetRotationEulerDegree(props.rotation_degrees);
  if (props.pivot != Vector3::Zero) SetPivot(props.pivot);
  if (props.anchor != Vector3::Zero) SetAnchor(props.anchor);
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
  local_euler_degrees_ = quat.ToEulerAngles() * Math::ToDegrees(1.0f);
  is_dirty_ = true;
}

void TransformComponent::SetRotationEuler(float pitch, float yaw, float roll) {
  local_rot_ = Quaternion::CreateFromEulerAngles(pitch, yaw, roll);
  local_euler_degrees_ = {Math::ToDegrees(pitch), Math::ToDegrees(yaw), Math::ToDegrees(roll)};
  is_dirty_ = true;
}

void TransformComponent::SetRotationEuler(const Vector3& euler) {
  SetRotationEuler(euler.x, euler.y, euler.z);
}

void TransformComponent::SetRotationEulerDegree(float pitch, float yaw, float roll) {
  local_rot_ = Quaternion::CreateFromEulerAngles(Math::ToRadians(pitch), Math::ToRadians(yaw), Math::ToRadians(roll));
  local_euler_degrees_ = {pitch, yaw, roll};
  is_dirty_ = true;
}

void TransformComponent::SetRotationEulerDegree(const Vector3& euler) {
  SetRotationEulerDegree(euler.x, euler.y, euler.z);
}

void TransformComponent::SetPivot(const Vector3& pivot) {
  local_pivot_ = pivot;
  is_dirty_ = true;
}

void TransformComponent::SetAnchor(const Vector3& anchor) {
  local_anchor_ = anchor;
  is_dirty_ = true;
}

void TransformComponent::UpdateLocalMatrix() {
  if (local_pivot_ == Vector3::Zero && local_anchor_ == Vector3::Zero) {
    local_matrix_ = Matrix4::CreateFromTRS(local_pos_, local_rot_, local_scale_);
    return;
  }

  // M = T(-pivot - anchor) * S * R * T(pivot + pos)
  // v_out = (v - pivot - anchor) * S * R + pivot + pos
  Matrix4 pre_translate = Matrix4::CreateTranslation(-local_pivot_ - local_anchor_);
  Matrix4 scale = Matrix4::CreateScale(local_scale_);
  Matrix4 rotation = Matrix4::CreateFromQuaternion(local_rot_);
  Matrix4 post_translate = Matrix4::CreateTranslation(local_pivot_ + local_pos_);

  local_matrix_ = pre_translate * scale * rotation * post_translate;
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

Vector3 TransformComponent::GetWorldPosition() {
  return GetWorldMatrix().GetTranslation();
}

Vector3 TransformComponent::GetWorldScale() {
  return GetWorldMatrix().GetScale();
}

#if ENABLE_EDITOR
void TransformComponent::OnInspectorGUI() {
  auto& snap = GetSnapConfig();

  Vector3 position = GetPosition();
  if (editor_ui::DragFloat3("Position", &position.x, snap.position)) SetPosition(position);

  Vector3 euler_deg = GetRotationDegrees();
  auto wrap = [](float deg) { return std::fmod(std::fmod(deg, 360.0f) + 540.0f, 360.0f) - 180.0f; };
  Vector3 display_deg = {wrap(euler_deg.x), wrap(euler_deg.y), wrap(euler_deg.z)};
  if (editor_ui::DragFloat3("Rotation", &display_deg.x, snap.rotation)) {
    Vector3 delta = display_deg - Vector3{wrap(euler_deg.x), wrap(euler_deg.y), wrap(euler_deg.z)};
    SetRotationEulerDegree(euler_deg + delta);
  }

  Vector3 scale = GetScale();
  if (editor_ui::DragFloat3("Scale", &scale.x, snap.scale)) SetScale(scale);

  Vector3 pivot = GetPivot();
  if (editor_ui::DragFloat3("Pivot", &pivot.x, snap.position)) SetPivot(pivot);

  Vector3 anchor = GetAnchor();
  if (editor_ui::DragFloat3("Anchor", &anchor.x, snap.position)) SetAnchor(anchor);
}
#endif
