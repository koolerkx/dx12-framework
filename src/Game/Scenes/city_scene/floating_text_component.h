#pragma once

#include "Component/Renderer/text_renderer.h"
#include "Component/behavior_component.h"
#include "Component/transform_component.h"
#include "Framework/Math/Math.h"
#include "game_object.h"

class FloatingTextComponent : public BehaviorComponent<FloatingTextComponent> {
 public:
  struct Props {
    float speed = 2.0f;
    float visible_duration = 1.0f;
    float fade_duration = 0.3f;
    Math::Vector4 color = {1.0f, 1.0f, 1.0f, 1.0f};
  };

  using BehaviorComponent::BehaviorComponent;
  FloatingTextComponent(GameObject* owner, const Props& props)
      : BehaviorComponent(owner),
        speed_(props.speed),
        visible_duration_(props.visible_duration),
        fade_duration_(props.fade_duration),
        color_(props.color) {
  }

  void OnUpdate(float dt) override {
    elapsed_ += dt;

    auto* transform = GetOwner()->GetTransform();
    auto pos = transform->GetWorldPosition();
    pos.y += speed_ * dt;
    transform->SetPosition(pos);

    float total_lifetime = visible_duration_ + fade_duration_;
    if (elapsed_ >= total_lifetime) {
      GetOwner()->Destroy();
      return;
    }

    if (elapsed_ > visible_duration_) {
      float fade_elapsed = elapsed_ - visible_duration_;
      float alpha = 1.0f - (fade_elapsed / fade_duration_);
      UpdateTextAlpha(alpha);
    }
  }

 private:
  void UpdateTextAlpha(float alpha) {
    auto* text = GetOwner()->GetComponent<TextRenderer>();
    if (!text) return;
    Math::Vector4 c = color_;
    c.w = alpha;
    text->SetColor(c);
  }

  float speed_ = 2.0f;
  float visible_duration_ = 1.0f;
  float fade_duration_ = 0.3f;
  Math::Vector4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};
  float elapsed_ = 0.0f;
};
