#pragma once

#include <algorithm>

#include "Component/behavior_component.h"
#include "Graphic/graphic.h"

class ScreenEffectController : public BehaviorComponent<ScreenEffectController> {
 public:
  using BehaviorComponent::BehaviorComponent;

  void TriggerChromaticAberration(float intensity) {
    GetContext()->GetGraphic()->GetChromaticAberrationConfig().intensity = intensity;
  }

  void OnUpdate(float dt) override {
    auto& config = GetContext()->GetGraphic()->GetChromaticAberrationConfig();
    if (config.intensity > 0.0f) {
      config.intensity = (std::max)(0.0f, config.intensity - DECAY_RATE * dt);
    }
  }

 private:
  static constexpr float DECAY_RATE = 3.0f;
};
