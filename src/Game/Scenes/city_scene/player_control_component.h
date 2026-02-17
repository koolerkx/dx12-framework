#pragma once

#include <cstdint>

#include "Component/behavior_component.h"
#include "Framework/Event/event_scope.hpp"
#include "Graphic/Pipeline/pixel_shader_descriptors.h"

class TowerPlacementComponent;
class PulsePathComponent;
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

  Graphics::PathPulseShader::Params pulse_params_ = {
    .pulse_r = 1.0f,
    .pulse_g = 0.15f,
    .pulse_b = 0.1f,
    .emissive_intensity = 8.0f,
    .pulse_speed = 1.5f,
    .pulse_frequency = 0.4f,
    .pulse_width = 1.0f,
  };
};
