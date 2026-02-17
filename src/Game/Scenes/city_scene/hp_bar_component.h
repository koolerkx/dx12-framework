#pragma once

#include "Component/behavior_component.h"

class SpriteRenderer;
class EnemyComponent;

class HpBarComponent : public BehaviorComponent<HpBarComponent> {
 public:
  struct Props {
    float bar_width = 1.0f;
    float y_offset = 1.2f;
  };

  using BehaviorComponent::BehaviorComponent;
  HpBarComponent(GameObject* owner, const Props& props);

  void OnStart() override;
  void OnUpdate(float dt) override;

 private:
  float bar_width_;
  float y_offset_;
  float fill_width_ = 0.0f;
  float fill_height_ = 0.0f;

  EnemyComponent* enemy_ = nullptr;
  SpriteRenderer* main_renderer_ = nullptr;
  float max_hp_ = 1.0f;
};
