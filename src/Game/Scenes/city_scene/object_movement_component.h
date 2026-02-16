#pragma once

#include <vector>

#include "Component/behavior_component.h"
#include "Framework/Math/Math.h"

class NavGrid;

class ObjectMovementComponent : public BehaviorComponent<ObjectMovementComponent> {
 public:
  struct Props {
    NavGrid* nav = nullptr;
    float move_speed = 3.0f;
    float waypoint_reach_threshold = 0.15f;
    Math::Vector2 initial_target_xz = {};
    bool has_initial_target = false;
  };

  ObjectMovementComponent(GameObject* owner, const Props& props);

  void OnInit() override;
  void OnStart() override;
  void OnReset() override;
  void OnUpdate(float dt) override;
  void OnDebugDraw(DebugDrawer& drawer) override;

  void MoveToXZ(Math::Vector2 target_xz);
  bool IsMoving() const;

 private:
  void MoveAlongPath(float dt);
  void ResetToSpawn();

  NavGrid* nav_ = nullptr;
  float move_speed_ = 3.0f;
  float waypoint_reach_threshold_ = 0.15f;
  float agent_radius_ = 0.0f;
  Math::Vector2 initial_target_xz_ = {};
  bool has_initial_target_ = false;

  Math::Vector3 spawn_position_ = {};
  std::vector<Math::Vector2> waypoints_;
  size_t current_waypoint_ = 0;
  bool moving_ = false;
};
