#pragma once

#include "Component/behavior_component.h"
#include "Framework/Event/event_scope.hpp"
#include "Framework/Math/Math.h"

class GameObject;

class ProjectileComponent : public BehaviorComponent<ProjectileComponent> {
 public:
  struct Props {
    GameObject* target = nullptr;
    float speed = 8.0f;
    float damage = 1.0f;
    float hit_radius = 0.5f;
    float life_time = 3.0f;
  };

  using BehaviorComponent::BehaviorComponent;
  ProjectileComponent(GameObject* owner, const Props& props)
      : BehaviorComponent(owner),
        target_(props.target),
        speed_(props.speed),
        damage_(props.damage),
        hit_radius_(props.hit_radius),
        life_time_(props.life_time) {
  }

  void OnStart() override;
  void OnUpdate(float dt) override;

 private:
  void SpawnBulletHitExplosion(const Math::Vector3& hit_position);

  GameObject* target_ = nullptr;
  float speed_ = 8.0f;
  float damage_ = 1.0f;
  float hit_radius_ = 0.5f;
  float life_time_ = 3.0f;
  bool is_running_ = true;
  EventScope event_scope_;
};
