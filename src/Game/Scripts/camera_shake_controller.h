#pragma once

#include <random>

#include "Component/behavior_component.h"
#include "Component/transform_component.h"

#if defined(DEBUG) || defined(_DEBUG)
#include "Framework/Input/keyboard.h"
#include "Framework/Render/render_service.h"
#include "Scenes/city_scene/city_scene_config.h"
#endif

class CameraShakeController : public BehaviorComponent<CameraShakeController> {
 public:
  using BehaviorComponent::BehaviorComponent;

  void Trigger(float intensity, float duration) {
    if (duration <= 0.0f) return;
    original_anchor_ = GetOwner()->GetTransform()->GetAnchor();
    intensity_ = intensity;
    duration_ = duration;
    remaining_ = duration;
  }

  void OnUpdate(float dt) override {
#if defined(DEBUG) || defined(_DEBUG)
    auto* input = GetContext()->GetInput();
    if (input && input->GetKeyDown(Keyboard::KeyCode::O)) {
      const CitySceneConfig::ArrivalScreenEffectConfig fx_cfg;
      Trigger(fx_cfg.shake_intensity, fx_cfg.shake_duration);
      GetContext()->GetRenderService()->SetChromaticAberrationIntensity(fx_cfg.chromatic_aberration_intensity);
    }
#endif

    if (remaining_ <= 0.0f) return;

    remaining_ -= dt;
    if (remaining_ <= 0.0f) {
      remaining_ = 0.0f;
      GetOwner()->GetTransform()->SetAnchor(original_anchor_);
      return;
    }

    float decay = remaining_ / duration_;
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    float offset_x = dist(rng_) * intensity_ * decay;
    float offset_y = dist(rng_) * intensity_ * decay;

    auto anchor = original_anchor_;
    anchor.x += offset_x;
    anchor.y += offset_y;
    GetOwner()->GetTransform()->SetAnchor(anchor);
  }

 private:
  float intensity_ = 0.0f;
  float duration_ = 0.0f;
  float remaining_ = 0.0f;
  Math::Vector3 original_anchor_ = Math::Vector3::Zero;
  std::mt19937 rng_{std::random_device{}()};
};
