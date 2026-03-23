#pragma once

#include <cmath>

#include "Component/behavior_component.h"
#include "Component/transform_component.h"
#include "Framework/Event/event_scope.hpp"
#include "Scenes/city_scene/city_scene_events.h"
#include "Scenes/city_scene/object_movement_component.h"
#include "game_context.h"
#include "game_object.h"
#include "scene_events.h"

class EnemyComponent : public BehaviorComponent<EnemyComponent> {
 public:
  struct Props {
    float contact_damage = 1.0f;
    float bob_amplitude = 0.15f;
    float bob_speed = 2.0f;
    int kill_reward = 0;
  };

  using BehaviorComponent::BehaviorComponent;
  EnemyComponent(GameObject* owner, const Props& props)
      : BehaviorComponent(owner),
        contact_damage_(props.contact_damage),
        bob_amplitude_(props.bob_amplitude),
        bob_speed_(props.bob_speed),
        kill_reward_(props.kill_reward) {
  }

  void OnStart() override {
    mesh_go_ = GetOwner()->FindChild(GetOwner()->GetName() + "_Mesh");
    auto* bus = GetContext()->GetEventBus().get();
    event_scope_.Subscribe<GameOverEvent>(*bus, [this](const GameOverEvent&) { is_running_ = false; });
  }

  void OnUpdate(float dt) override {
    if (!is_running_) return;

    auto* movement = GetOwner()->GetComponent<ObjectMovementComponent>();
    if (movement && was_moving_ && !movement->IsMoving()) {
      auto pos = GetOwner()->GetTransform()->GetWorldPosition();
      GetContext()->GetEventBus()->Emit(EnemyArrivedEvent{.position = pos});
      GetOwner()->Destroy();
      return;
    }
    if (movement) was_moving_ = movement->IsMoving();

    if (!mesh_go_) return;
    bob_time_ += dt;
    auto* transform = mesh_go_->GetTransform();
    auto anchor = transform->GetAnchor();
    anchor.y = std::sin(bob_time_ * bob_speed_) * bob_amplitude_;
    transform->SetAnchor(anchor);
  }

  void AwardKillReward() {
    if (kill_reward_ <= 0) return;
    auto pos = GetOwner()->GetTransform()->GetWorldPosition();
    GetContext()->GetEventBus()->Emit(EntityDeathEvent{
      .kill_reward = kill_reward_,
      .position = pos,
    });
  }

  float GetContactDamage() const {
    return contact_damage_;
  }

 private:
  float contact_damage_ = 1.0f;
  float bob_amplitude_ = 0.15f;
  float bob_speed_ = 2.0f;
  float bob_time_ = 0.0f;
  int kill_reward_ = 0;
  bool is_running_ = true;
  bool was_moving_ = false;
  EventScope event_scope_;
  GameObject* mesh_go_ = nullptr;
};
