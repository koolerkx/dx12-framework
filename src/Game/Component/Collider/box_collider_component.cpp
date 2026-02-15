#include "box_collider_component.h"

#include "Debug/debug_drawer.h"
#include "Framework/Core/color.h"

BoxColliderComponent::BoxColliderComponent(GameObject* owner, const Math::AABB& local_bounds, const Math::Matrix4& world_matrix)
    : ColliderComponent(owner), local_bounds_(local_bounds), world_matrix_(world_matrix) {
}

ComponentTypeID BoxColliderComponent::GetTypeID() const {
  return ComponentType<BoxColliderComponent>::GetID();
}

const char* BoxColliderComponent::GetTypeName() const {
  return "BoxColliderComponent";
}

Math::AABB BoxColliderComponent::GetWorldBounds() const {
  using Math::Vector3;

  Vector3 corners[8] = {
    {local_bounds_.min.x, local_bounds_.min.y, local_bounds_.min.z},
    {local_bounds_.max.x, local_bounds_.min.y, local_bounds_.min.z},
    {local_bounds_.min.x, local_bounds_.max.y, local_bounds_.min.z},
    {local_bounds_.max.x, local_bounds_.max.y, local_bounds_.min.z},
    {local_bounds_.min.x, local_bounds_.min.y, local_bounds_.max.z},
    {local_bounds_.max.x, local_bounds_.min.y, local_bounds_.max.z},
    {local_bounds_.min.x, local_bounds_.max.y, local_bounds_.max.z},
    {local_bounds_.max.x, local_bounds_.max.y, local_bounds_.max.z},
  };

  Vector3 world_min = world_matrix_.TransformPoint(corners[0]);
  Vector3 world_max = world_min;

  for (int i = 1; i < 8; ++i) {
    Vector3 transformed = world_matrix_.TransformPoint(corners[i]);
    world_min = Vector3::Min(world_min, transformed);
    world_max = Vector3::Max(world_max, transformed);
  }

  return Math::AABB(world_min, world_max);
}

void BoxColliderComponent::OnDebugDraw(DebugDrawer& drawer) {
  Math::AABB world_bounds = GetWorldBounds();
  drawer.DrawWireCube(world_bounds.min, world_bounds.max, colors::Green);
}
