#pragma once

#include <cstdint>

#include "Component/behavior_component.h"
#include "Framework/Event/event_scope.hpp"
#include "Scenes/city_scene/pulse_path_component.h"

class TowerPlacementComponent;
class InputSystem;
class NavGrid;
class GameObject;

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
  void UpdateEnemyHover();
  void ClearEnemyHover();

  EventScope event_scope_;
  NavGrid* nav_ = nullptr;
  PlayerMode mode_ = PlayerMode::Normal;
  InputSystem* input_ = nullptr;
  PulsePathComponent* pulse_path_ = nullptr;
  GameObject* hovered_enemy_ = nullptr;

  PulseCB pulse_params_ = [] {
    PulseCB params{};
    params.pulseColor = {1.0f, 0.15f, 0.1f};
    params.emissiveIntensity = 8.0f;
    params.pulseSpeed = 1.5f;
    params.pulseFrequency = 0.4f;
    params.pulseWidth = 1.0f;
    return params;
  }();
};
