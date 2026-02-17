#pragma once

#include <memory>

#include "Component/behavior_component.h"

struct ModelData;
class GameObject;

class TowerComponent : public BehaviorComponent<TowerComponent> {
 public:
  struct Props {
    float range = 5.0f;
    float shoot_interval = 1.0f;
    float damage = 1.0f;
  };

  using BehaviorComponent::BehaviorComponent;
  TowerComponent(GameObject* owner, const Props& props)
      : BehaviorComponent(owner), range_(props.range), shoot_interval_(props.shoot_interval), damage_(props.damage) {
  }

  void OnStart() override;
  void OnUpdate(float dt) override;

 private:
  GameObject* FindNearestEnemy() const;
  void SpawnProjectile(GameObject* target);

  float range_ = 5.0f;
  float shoot_interval_ = 1.0f;
  float damage_ = 1.0f;
  float shoot_timer_ = 0.0f;
  GameObject* enemy_manager_ = nullptr;
  std::shared_ptr<ModelData> projectile_model_;
};
