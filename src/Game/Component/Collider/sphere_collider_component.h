#pragma once

#include "collider_component.h"

class SphereColliderComponent : public ColliderComponent {
 public:
  using ColliderComponent::ColliderComponent;
  SphereColliderComponent(GameObject* owner, float radius);

  ComponentTypeID GetTypeID() const override;
  const char* GetTypeName() const override;

  Math::AABB GetWorldBounds() const override;
  Math::Sphere GetWorldSphere() const;
  void OnDebugDraw(DebugDrawer& drawer) override;

 private:
  float radius_;
};
