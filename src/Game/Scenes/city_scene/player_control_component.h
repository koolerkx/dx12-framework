#pragma once

#include <cstdint>

#include "Component/behavior_component.h"

class TowerPlacementComponent;
class InputSystem;
class NavGrid;

enum class PlayerMode : uint8_t { Normal, PlacingTower };

class PlayerControlComponent : public BehaviorComponent<PlayerControlComponent> {
 public:
  struct Props {
    NavGrid* nav = nullptr;
  };

  using BehaviorComponent::BehaviorComponent;
  PlayerControlComponent(GameObject* owner, const Props& props) : BehaviorComponent(owner), nav_(props.nav) {
  }

  void OnInit() override;
  void OnUpdate(float dt) override;

 private:
  void EnterPlacingMode();
  void ReturnToNormal();

  NavGrid* nav_ = nullptr;
  PlayerMode mode_ = PlayerMode::Normal;
  InputSystem* input_ = nullptr;
};
