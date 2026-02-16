#pragma once

#include "Component/behavior_component.h"
#include "game_object.h"

class EnemyComponent : public BehaviorComponent<EnemyComponent> {
 public:
  struct Props {
    float hp = 2.0f;
    float contact_damage = 1.0f;
  };

  using BehaviorComponent::BehaviorComponent;
  EnemyComponent(GameObject* owner, const Props& props) : BehaviorComponent(owner), hp_(props.hp), contact_damage_(props.contact_damage) {
  }

  void TakeDamage(float amount) {
    hp_ -= amount;
    if (hp_ <= 0.0f) {
      GetOwner()->Destroy();
    }
  }

  float GetHP() const {
    return hp_;
  }
  float GetContactDamage() const {
    return contact_damage_;
  }

 private:
  float hp_ = 2.0f;
  float contact_damage_ = 1.0f;
};
