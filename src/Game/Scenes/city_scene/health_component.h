#pragma once

#include <functional>

#include "Component/behavior_component.h"

class HealthComponent : public BehaviorComponent<HealthComponent> {
 public:
  struct Props {
    float max_hp = 1.0f;
    std::function<void()> on_health_depleted;
  };

  using BehaviorComponent::BehaviorComponent;
  HealthComponent(GameObject* owner, const Props& props)
      : BehaviorComponent(owner), hp_(props.max_hp), max_hp_(props.max_hp), on_health_depleted_(std::move(props.on_health_depleted)) {
  }

  void TakeDamage(float amount) {
    if (dead_) return;
    hp_ -= amount;
    if (hp_ <= 0.0f) {
      hp_ = 0.0f;
      dead_ = true;
      if (on_health_depleted_) on_health_depleted_();
    }
  }

  float GetHP() const {
    return hp_;
  }
  float GetMaxHP() const {
    return max_hp_;
  }
  bool IsDead() const {
    return dead_;
  }

 private:
  float hp_;
  float max_hp_;
  bool dead_ = false;
  std::function<void()> on_health_depleted_;
};
