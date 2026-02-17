#pragma once

#include <cmath>

#include "Component/behavior_component.h"
#include "Component/transform_component.h"
#include "game_object.h"

class EnemyComponent : public BehaviorComponent<EnemyComponent> {
 public:
  struct Props {
    float hp = 2.0f;
    float contact_damage = 1.0f;
    float bob_amplitude = 0.15f;
    float bob_speed = 2.0f;
  };

  using BehaviorComponent::BehaviorComponent;
  EnemyComponent(GameObject* owner, const Props& props)
    : BehaviorComponent(owner),
      hp_(props.hp),
      max_hp_(props.hp),
      contact_damage_(props.contact_damage),
      bob_amplitude_(props.bob_amplitude),
      bob_speed_(props.bob_speed) {
  }

  void OnStart() override {
    mesh_go_ = GetOwner()->FindChild(GetOwner()->GetName() + "_Mesh");
  }

  void OnUpdate(float dt) override {
    if (!mesh_go_) return;
    bob_time_ += dt;
    auto* transform = mesh_go_->GetTransform();
    auto anchor = transform->GetAnchor();
    anchor.y = std::sin(bob_time_ * bob_speed_) * bob_amplitude_;
    transform->SetAnchor(anchor);
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
  float GetMaxHP() const {
    return max_hp_;
  }
  float GetContactDamage() const {
    return contact_damage_;
  }

 private:
  float hp_ = 2.0f;
  float max_hp_ = 2.0f;
  float contact_damage_ = 1.0f;
  float bob_amplitude_ = 0.15f;
  float bob_speed_ = 2.0f;
  float bob_time_ = 0.0f;
  GameObject* mesh_go_ = nullptr;
};
