#pragma once

#include "Component/behavior_component.h"

class BaseHealthComponent : public BehaviorComponent<BaseHealthComponent> {
 public:
  struct Props {};

  using BehaviorComponent::BehaviorComponent;
  BaseHealthComponent(GameObject* owner, const Props& props);

  void OnStart() override;

  void TakeDamage(int amount = 1);
  int GetHealth() const { return health_; }

 private:
  void SpawnBaseDestroyedEffect();

  int health_ = 0;
};
