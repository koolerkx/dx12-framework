#pragma once

#include <algorithm>

#include "Component/behavior_component.h"
#include "Framework/Render/render_service.h"

class ScreenEffectController : public BehaviorComponent<ScreenEffectController> {
 public:
  using BehaviorComponent::BehaviorComponent;

  void TriggerChromaticAberration(float intensity) {
    GetContext()->GetRenderService()->SetChromaticAberrationIntensity(intensity);
  }

  void OnUpdate(float dt) override {
    auto* rs = GetContext()->GetRenderService();
    float current = rs->GetChromaticAberrationIntensity();
    if (current > 0.0f) {
      rs->SetChromaticAberrationIntensity((std::max)(0.0f, current - DECAY_RATE * dt));
    }
  }

 private:
  static constexpr float DECAY_RATE = 3.0f;
};
