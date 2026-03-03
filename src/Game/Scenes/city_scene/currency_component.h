#pragma once

#include "Component/behavior_component.h"

class CurrencyComponent : public BehaviorComponent<CurrencyComponent> {
 public:
  struct Props {};

  using BehaviorComponent::BehaviorComponent;
  CurrencyComponent(GameObject* owner, const Props& props);

  void OnStart() override;

  void AddGold(int amount);
  bool TrySpendGold(int amount);
  int GetGold() const {
    return gold_;
  }

 private:
  int gold_ = 0;
};
