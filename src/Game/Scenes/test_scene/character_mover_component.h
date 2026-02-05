#pragma once
#include <DirectXMath.h>

#include "Component/behavior_component.h"

class CharacterMover : public BehaviorComponent<CharacterMover> {
 public:
  using BehaviorComponent::BehaviorComponent;

  void OnUpdate(float dt) override;  // Game logic here!

 private:
  float time_ = 0.0f;
};
