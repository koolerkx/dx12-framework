#include "sphere_collider_component.h"

#include "Component/transform_component.h"
#include "Debug/debug_drawer.h"
#include "Framework/Core/color.h"
#include "game_object.h"

SphereColliderComponent::SphereColliderComponent(GameObject* owner, float radius)
    : ColliderComponent(owner), radius_(radius) {}

ComponentTypeID SphereColliderComponent::GetTypeID() const {
  return ComponentType<SphereColliderComponent>::GetID();
}

const char* SphereColliderComponent::GetTypeName() const {
  return "SphereColliderComponent";
}

Math::AABB SphereColliderComponent::GetWorldBounds() const {
  auto sphere = GetWorldSphere();
  Math::Vector3 extent(sphere.radius, sphere.radius, sphere.radius);
  return {sphere.center - extent, sphere.center + extent};
}

Math::Sphere SphereColliderComponent::GetWorldSphere() const {
  auto* transform = GetOwner()->GetTransform();
  Math::Vector3 center = transform->GetWorldPosition();
  Math::Vector3 scale = transform->GetScale();
  float max_scale = (std::max)({scale.x, scale.y, scale.z});
  return {center, radius_ * max_scale};
}

void SphereColliderComponent::OnDebugDraw(DebugDrawer& drawer) {
  auto sphere = GetWorldSphere();
  drawer.DrawWireSphere(sphere.center, sphere.radius, colors::Green);
}
