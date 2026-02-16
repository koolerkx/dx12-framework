#pragma once

#include <cstdint>

#include "Component/behavior_component.h"

class TowerPlacementComponent;
class InputSystem;

enum class PlayerMode : uint8_t { Normal, PlacingTower };

class PlayerControlComponent : public BehaviorComponent<PlayerControlComponent> {
 public:
  using BehaviorComponent::BehaviorComponent;

  void OnInit() override;
  void OnUpdate(float dt) override;

 private:
  void EnterPlacingMode();
  void ReturnToNormal();

  PlayerMode mode_ = PlayerMode::Normal;
  InputSystem* input_ = nullptr;
};
