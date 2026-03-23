#pragma once
#include "Component/behavior_component.h"

class CharacterMover : public BehaviorComponent<CharacterMover> {
 public:
  using BehaviorComponent::BehaviorComponent;

  void OnUpdate(float dt) override;

 private:
  float time_ = 0.0f;
};
