#include "Scenes/city_scene/enemy_spawn_manager_component.h"

#include <string>

#include "Asset/asset_manager.h"
#include "Component/Collider/sphere_collider_component.h"
#include "Component/enemy_spawn_component.h"
#include "Component/model_component.h"
#include "Component/player_spawn_component.h"
#include "Component/transform_component.h"
#include "Scenes/city_scene/city_scene_config.h"
#include "Scenes/city_scene/enemy_component.h"
#include "Scenes/city_scene/hp_bar_component.h"
#include "Scenes/city_scene/object_movement_component.h"
#include "game_context.h"
#include "game_object.h"
#include "scene.h"

namespace cfg = CitySceneConfig;

EnemySpawnManagerComponent::EnemySpawnManagerComponent(GameObject* owner, const Props& props) : BehaviorComponent(owner), nav_(props.nav) {
}

void EnemySpawnManagerComponent::OnStart() {
  auto& assets = GetContext()->GetAssetManager();
  enemy_model_ = assets.LoadModel(cfg::PATHS.enemy_model, cfg::FBX_UNIT_SCALE);

  auto* scene = GetOwner()->GetScene();
  for (const auto& go : scene->GetGameObjects()) {
    if (go->GetComponent<EnemySpawnComponent>()) {
      enemy_spawners_.push_back(go.get());
    }
    if (go->GetComponent<PlayerSpawnComponent>()) {
      auto* transform = go->GetTransform();
      auto pos = transform->GetWorldPosition();
      player_spawn_xz_ = {pos.x, pos.z};
    }
  }
}

void EnemySpawnManagerComponent::OnReset() {
  enemy_spawners_.clear();
  enemy_model_.reset();
  enemy_counter_ = 0;
}

void EnemySpawnManagerComponent::SpawnEnemy(int spawner_index, int wave_index) {
  if (spawner_index < 0 || spawner_index >= static_cast<int>(enemy_spawners_.size())) return;
  SpawnEnemyAt(enemy_spawners_[spawner_index], wave_index);
}

int EnemySpawnManagerComponent::GetSpawnerCount() const {
  return static_cast<int>(enemy_spawners_.size());
}

void EnemySpawnManagerComponent::SpawnEnemyAt(GameObject* spawner, int wave_index) {
  if (!enemy_model_) return;

  auto spawner_pos = spawner->GetTransform()->GetWorldPosition();
  Math::Vector3 spawn_pos = {spawner_pos.x, 0.5f, spawner_pos.z};

  std::string name = "Enemy_" + std::to_string(enemy_counter_++);
  auto* scene = GetOwner()->GetScene();

  const cfg::EnemyConfig ENEMY;
  float s = ENEMY.size_scale;
  auto* enemy = scene->CreateGameObject(name, {.position = spawn_pos, .scale = {s, s, s}});
  enemy->SetParent(GetOwner());

  auto* enemy_mesh = scene->CreateGameObject(name + "_Mesh");
  enemy_mesh->SetTransient(true);
  enemy_mesh->SetParent(enemy);
  enemy_mesh->AddComponent<ModelComponent>(ModelComponent::Props{.model = enemy_model_});

  auto extents = enemy_model_->bounds.GetExtents();
  float base_radius = (std::max)(extents.x, extents.z);
  enemy->AddComponent<SphereColliderComponent>(base_radius);
  enemy->AddComponent<EnemyComponent>(EnemyComponent::Props{.hp = ENEMY.ComputeHP(wave_index)});
  enemy->AddComponent<HpBarComponent>(HpBarComponent::Props{});
  enemy->AddComponent<ObjectMovementComponent>(ObjectMovementComponent::Props{
    .nav = nav_,
    .move_speed = ENEMY.move_speed,
    .agent_size_scale = ENEMY.agent_size_scale,
    .initial_target_xz = player_spawn_xz_,
    .has_initial_target = true,
  });
}
