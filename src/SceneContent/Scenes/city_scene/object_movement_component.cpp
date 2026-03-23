#include "object_movement_component.h"

#include "Component/Collider/box_collider_component.h"
#include "Component/Collider/sphere_collider_component.h"
#include "Component/transform_component.h"
#include "Debug/debug_drawer.h"
#include "Framework/Core/color.h"
#include "Framework/Logging/logger.h"
#include "Map/nav_grid.h"
#include "Map/nav_grid_events.h"
#include "Map/pathfinder.h"
#include "game_context.h"
#include "game_object.h"
#include "scene_events.h"

using Math::Vector2;
using Math::Vector3;

ObjectMovementComponent::ObjectMovementComponent(GameObject* owner, const Props& props)
    : BehaviorComponent(owner),
      nav_(props.nav),
      move_speed_(props.move_speed),
      waypoint_reach_threshold_(props.waypoint_reach_threshold),
      agent_size_scale_(props.agent_size_scale),
      initial_target_xz_(props.initial_target_xz),
      has_initial_target_(props.has_initial_target) {
}

void ObjectMovementComponent::OnInit() {
  spawn_position_ = GetOwner()->GetTransform()->GetPosition();

  auto* sphere = GetOwner()->GetComponent<SphereColliderComponent>();
  if (sphere) {
    agent_radius_ = sphere->GetWorldSphere().radius * agent_size_scale_;
  }

  auto* bus = GetContext()->GetEventBus().get();
  event_scope_.Subscribe<NavGridChangedEvent>(*bus, [this](const NavGridChangedEvent& event) {
    if (!moving_) return;
    if (!IsPathAffected(event.affected_area)) return;
    MoveToXZ(goal_xz_);
  });
  event_scope_.Subscribe<GameOverEvent>(*bus, [this](const GameOverEvent&) {
    is_running_ = false;
  });
}

void ObjectMovementComponent::OnStart() {
  if (has_initial_target_) {
    MoveToXZ(initial_target_xz_);
  }
}

void ObjectMovementComponent::OnReset() {
  ResetToSpawn();
  if (has_initial_target_) {
    MoveToXZ(initial_target_xz_);
  }
}

void ObjectMovementComponent::OnUpdate(float dt) {
  if (!is_running_) return;
  if (moving_) {
    MoveAlongPath(dt);
  }
}

void ObjectMovementComponent::OnDebugDraw(DebugDrawer& drawer) {
  if (waypoints_.empty()) return;

  constexpr float PATH_Y = 0.2f;

  for (size_t i = current_waypoint_; i + 1 < waypoints_.size(); ++i) {
    Vector3 a = {waypoints_[i].x, PATH_Y, waypoints_[i].y};
    Vector3 b = {waypoints_[i + 1].x, PATH_Y, waypoints_[i + 1].y};
    drawer.DrawLine(a, b, colors::Yellow);
  }

  if (!waypoints_.empty()) {
    auto& goal = waypoints_.back();
    drawer.DrawWireSphere({goal.x, PATH_Y, goal.y}, 0.3f, colors::Lime, 8);
  }
}

void ObjectMovementComponent::MoveToXZ(Vector2 target_xz) {
  goal_xz_ = target_xz;
  waypoints_.clear();
  current_waypoint_ = 0;
  moving_ = false;

  if (!nav_) return;

  auto* transform = GetOwner()->GetTransform();
  if (!transform) return;

  Vector3 pos = transform->GetWorldPosition();
  auto result = Pathfinder::FindPath(*nav_, {pos.x, pos.z}, target_xz, agent_radius_);
  if (!result.found || result.waypoints.empty()) {
    Logger::LogFormat(LogLevel::Warn, LogCategory::Game, Logger::Here(),
      "Path not found: from ({:.2f}, {:.2f}) to ({:.2f}, {:.2f}), agent_radius={:.3f}",
      pos.x, pos.z, target_xz.x, target_xz.y, agent_radius_);
    return;
  }

  waypoints_ = std::move(result.waypoints);
  current_waypoint_ = 0;
  moving_ = true;
}

bool ObjectMovementComponent::IsMoving() const {
  return moving_;
}

void ObjectMovementComponent::ResetToSpawn() {
  auto* transform = GetOwner()->GetTransform();
  if (!transform) return;

  transform->SetPosition(spawn_position_);
  waypoints_.clear();
  current_waypoint_ = 0;
  moving_ = false;

  auto* collider = GetOwner()->GetComponent<BoxColliderComponent>();
  if (collider) {
    collider->SetWorldMatrix(transform->GetWorldMatrix());
  }
}

bool ObjectMovementComponent::IsPathAffected(const Math::AABB& area) const {
  for (size_t i = current_waypoint_; i < waypoints_.size(); ++i) {
    float wx = waypoints_[i].x;
    float wz = waypoints_[i].y;
    if (wx >= area.min.x && wx <= area.max.x &&
        wz >= area.min.z && wz <= area.max.z) {
      return true;
    }
  }
  return false;
}

void ObjectMovementComponent::MoveAlongPath(float dt) {
  if (current_waypoint_ >= waypoints_.size()) {
    moving_ = false;
    return;
  }

  auto* transform = GetOwner()->GetTransform();
  if (!transform) return;

  Vector3 pos = transform->GetPosition();
  Vector2 target_wp = waypoints_[current_waypoint_];
  Vector2 current_xz = {pos.x, pos.z};

  Vector2 diff = target_wp - current_xz;
  float distance = diff.Length();

  if (distance < waypoint_reach_threshold_) {
    ++current_waypoint_;
    if (current_waypoint_ >= waypoints_.size()) {
      moving_ = false;
    }
    return;
  }

  Vector2 direction = diff / distance;
  float step = move_speed_ * dt;
  if (step > distance) step = distance;

  Vector2 new_xz = current_xz + direction * step;
  transform->SetPosition({new_xz.x, pos.y, new_xz.y});

  float yaw_rad = std::atan2(direction.x, direction.y);
  float yaw_deg = DirectX::XMConvertToDegrees(yaw_rad);
  transform->SetRotationEulerDegree(0.0f, yaw_deg, 0.0f);

  auto* collider = GetOwner()->GetComponent<BoxColliderComponent>();
  if (collider) {
    collider->SetWorldMatrix(transform->GetWorldMatrix());
  }
}
