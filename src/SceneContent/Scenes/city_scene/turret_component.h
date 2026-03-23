#pragma once

#include <cstdint>

#include "Component/behavior_component.h"
#include "Framework/Event/event_scope.hpp"
#include "Framework/Math/Math.h"

class GameObject;

enum class LaserVisibility : uint8_t { AlwaysInRange, HighlightedOnly };

class TurretComponent : public BehaviorComponent<TurretComponent> {
 public:
  struct Props {
    float range = 3.0f;
    float shoot_interval = 1.0f;
    float damage = 1.0f;
    LaserVisibility laser_visibility = LaserVisibility::HighlightedOnly;
  };

  using BehaviorComponent::BehaviorComponent;
  TurretComponent(GameObject* owner, const Props& props)
      : BehaviorComponent(owner),
        range_(props.range),
        shoot_interval_(props.shoot_interval),
        damage_(props.damage),
        laser_visibility_(props.laser_visibility) {
  }

  void OnStart() override;
  void OnUpdate(float dt) override;
  void OnDestroy() override;

  void SetHighlighted(bool highlighted) {
    highlighted_ = highlighted;
  }

  float GetRange() const {
    return range_;
  }
  float GetDamage() const {
    return damage_;
  }
  float GetShootInterval() const {
    return shoot_interval_;
  }

  void AdvanceShootTimer(float dt) {
    shoot_timer_ += dt;
  }
  void ResetShootTimer() {
    shoot_timer_ = 0.0f;
  }
  bool IsReadyToShoot() const {
    return shoot_timer_ >= shoot_interval_;
  }

  bool IsRunning() const {
    return is_running_;
  }
  bool IsHighlighted() const {
    return highlighted_;
  }

  void SetLaserTarget(const Math::Vector3& position) {
    has_laser_target_ = true;
    laser_target_position_ = position;
  }

  void ClearLaserTarget() {
    has_laser_target_ = false;
  }

 private:
  bool ShouldShowLaser() const;
  void UpdateLaser();
  void DestroyLaser();

  float range_ = 3.0f;
  float shoot_interval_ = 1.0f;
  float damage_ = 1.0f;
  float shoot_timer_ = 0.0f;
  LaserVisibility laser_visibility_ = LaserVisibility::HighlightedOnly;
  bool highlighted_ = false;
  bool is_running_ = true;
  bool has_laser_target_ = false;
  Math::Vector3 laser_target_position_;
  EventScope event_scope_;
  GameObject* laser_go_ = nullptr;
};
