#pragma once

#include <cmath>

#include "Component/behavior_component.h"
#include "Component/transform_component.h"
#include "Scenes/city_scene/city_scene_config.h"
#include "Scenes/city_scene/explosion_effect.h"
#include "Scenes/city_scene/game_state_manager_component.h"
#include "Scenes/city_scene/object_movement_component.h"
#include "Scripts/camera_shake_controller.h"
#include "Scripts/screen_effect_controller.h"
#include "game_object.h"
#include "scene.h"

class EnemyComponent : public BehaviorComponent<EnemyComponent> {
 public:
  struct Props {
    float hp = 2.0f;
    float contact_damage = 1.0f;
    float bob_amplitude = 0.15f;
    float bob_speed = 2.0f;
    int kill_reward = 0;
  };

  using BehaviorComponent::BehaviorComponent;
  EnemyComponent(GameObject* owner, const Props& props)
      : BehaviorComponent(owner),
        hp_(props.hp),
        max_hp_(props.hp),
        contact_damage_(props.contact_damage),
        bob_amplitude_(props.bob_amplitude),
        bob_speed_(props.bob_speed),
        kill_reward_(props.kill_reward) {
  }

  void OnStart() override {
    mesh_go_ = GetOwner()->FindChild(GetOwner()->GetName() + "_Mesh");
  }

  void OnUpdate(float dt) override {
    auto* movement = GetOwner()->GetComponent<ObjectMovementComponent>();
    if (movement && was_moving_ && !movement->IsMoving()) {
      SpawnArrivalExplosion();
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

  void TakeDamage(float amount) {
    if (dead_) return;
    hp_ -= amount;
    if (hp_ <= 0.0f) {
      dead_ = true;
      AwardKillReward();
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
  void AwardKillReward() {
    if (kill_reward_ <= 0) return;
    auto* scene = GetOwner()->GetScene();
    if (!scene) return;
    auto* player = scene->FindGameObject("Player");
    if (!player) return;
    if (auto* gold = player->GetComponent<GameStateManagerComponent>())
      gold->AddGold(kill_reward_);
  }

  void SpawnArrivalExplosion() {
    auto* scene = GetOwner()->GetScene();
    if (!scene) return;
    const CitySceneConfig::ArrivalExplosionConfig cfg;
    auto pos = GetOwner()->GetTransform()->GetWorldPosition();
    pos.y += cfg.y_offset;
    CitySceneEffect::SpawnExplosion(scene, pos, CitySceneEffect::FromArrivalConfig(cfg), "ArrivalExplosion");

    const CitySceneConfig::ExplosionSparksConfig sparks_cfg;
    CitySceneEffect::SpawnExplosionSparks(scene, pos, CitySceneEffect::FromExplosionSparksConfig(sparks_cfg), "ArrivalSparks");

    const CitySceneConfig::ArrivalScreenEffectConfig fx_cfg;
    auto* camera_go = scene->FindGameObject("MainCamera");
    if (camera_go) {
      if (auto* shake = camera_go->GetComponent<CameraShakeController>()) shake->Trigger(fx_cfg.shake_intensity, fx_cfg.shake_duration);
      if (auto* screen_fx = camera_go->GetComponent<ScreenEffectController>())
        screen_fx->TriggerChromaticAberration(fx_cfg.chromatic_aberration_intensity);
    }

    DamagePlayerSpawn(scene);
  }

  void DamagePlayerSpawn(IScene* scene) {
    auto* player = scene->FindGameObject("Player");
    if (!player) return;
    if (auto* state = player->GetComponent<GameStateManagerComponent>())
      state->TakeDamage();
  }

  float hp_ = 2.0f;
  float max_hp_ = 2.0f;
  float contact_damage_ = 1.0f;
  float bob_amplitude_ = 0.15f;
  float bob_speed_ = 2.0f;
  float bob_time_ = 0.0f;
  int kill_reward_ = 0;
  bool dead_ = false;
  bool was_moving_ = false;
  GameObject* mesh_go_ = nullptr;
};
