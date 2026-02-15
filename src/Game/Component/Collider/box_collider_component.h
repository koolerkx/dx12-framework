#pragma once

#include "collider_component.h"

class BoxColliderComponent : public ColliderComponent {
 public:
  using ColliderComponent::ColliderComponent;
  BoxColliderComponent(GameObject* owner, const Math::AABB& local_bounds, const Math::Matrix4& world_matrix);

  ComponentTypeID GetTypeID() const override;
  const char* GetTypeName() const override;

  Math::AABB GetWorldBounds() const override;
  void OnDebugDraw(DebugDrawer& drawer) override;

  void SetWorldMatrix(const Math::Matrix4& world_matrix) { world_matrix_ = world_matrix; }

 private:
  Math::AABB local_bounds_;
  Math::Matrix4 world_matrix_;
};
