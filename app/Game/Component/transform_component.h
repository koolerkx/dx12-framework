#pragma once
#include <DirectXMath.h>

#include "Component/component.h"

class TransformComponent : public Component<TransformComponent> {
 public:
  using Component::Component;  // Use base constructor

  void OnStart() override;          // Ini`tialize cache
  void OnParentChanged() override;  // Update cache when hierarchy changes

  // Setters (Mark dirty)
  void SetPosition(const DirectX::XMFLOAT3& pos);
  void SetScale(const DirectX::XMFLOAT3& scale);

  void SetRotation(const DirectX::XMFLOAT4& quat);
  void SetRotationEuler(float pitch, float yaw, float roll);
  void SetRotationEuler(const DirectX::XMFLOAT3& euler);

  void SetRotationEulerDegree(float pitch, float yaw, float roll);
  void SetRotationEulerDegree(const DirectX::XMFLOAT3& euler);

  // Getters
  DirectX::XMFLOAT3 GetPosition() const {
    return local_pos_;
  }

  // Calculates World Matrix (Cached & Recursive)
  DirectX::XMMATRIX GetWorldMatrix();

 private:
  void UpdateLocalMatrix();

 private:
  DirectX::XMFLOAT3 local_pos_ = {0.f, 0.f, 0.f};
  DirectX::XMFLOAT3 local_scale_ = {1.f, 1.f, 1.f};
  DirectX::XMFLOAT4 local_rot_ = {0.f, 0.f, 0.f, 1.f};  // Identity Quaternion

  DirectX::XMFLOAT4X4 local_matrix_;
  DirectX::XMFLOAT4X4 world_matrix_;

  bool is_dirty_ = true;

  // Optimization: Cache pointer to parent transform to avoid GetComponent every frame
  TransformComponent* parent_transform_ = nullptr;
};
