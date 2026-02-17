#include "tower_component.h"

#include "Asset/asset_manager.h"
#include "Component/model_component.h"
#include "Component/transform_component.h"
#include "Framework/Math/Math.h"
#include "Scenes/city_scene/city_scene_config.h"
#include "Scenes/city_scene/enemy_component.h"
#include "Scenes/city_scene/projectile_component.h"
#include "game_object.h"
#include "scene.h"

namespace cfg = CitySceneConfig;

using Math::Vector3;

void TowerComponent::OnStart() {
  enemy_manager_ = GetOwner()->GetScene()->FindGameObject("EnemyManager");
  projectile_model_ = GetContext()->GetAssetManager().LoadModel(cfg::PATHS.projectile_model, cfg::FBX_UNIT_SCALE);
}

void TowerComponent::OnUpdate(float dt) {
  if (!enemy_manager_) return;

  shoot_timer_ += dt;
  if (shoot_timer_ < shoot_interval_) return;

  auto* target = FindNearestEnemy();
  if (!target) return;

  SpawnProjectile(target);
  shoot_timer_ = 0.0f;
}

GameObject* TowerComponent::FindNearestEnemy() const {
  auto tower_pos = GetOwner()->GetTransform()->GetWorldPosition();

  GameObject* nearest = nullptr;
  float nearest_dist_sq = range_ * range_;

  for (auto* child : enemy_manager_->GetChildren()) {
    if (child->IsPendingDestroy()) continue;
    if (!child->GetComponent<EnemyComponent>()) continue;

    auto enemy_pos = child->GetTransform()->GetWorldPosition();
    float dist_sq = Vector3::DistanceSquared(tower_pos, enemy_pos);
    if (dist_sq < nearest_dist_sq) {
      nearest_dist_sq = dist_sq;
      nearest = child;
    }
  }

  return nearest;
}

void TowerComponent::SpawnProjectile(GameObject* target) {
  static int global_projectile_id = 0;

  auto* scene = GetOwner()->GetScene();
  auto tower_pos = GetOwner()->GetTransform()->GetWorldPosition();

  auto* projectile =
    scene->CreateGameObject("Projectile_" + std::to_string(global_projectile_id++), {.position = tower_pos});
  projectile->SetTransient(true);

  projectile->AddComponent<ModelComponent>(ModelComponent::Props{.model = projectile_model_});

  projectile->AddComponent<ProjectileComponent>(ProjectileComponent::Props{
    .target = target,
    .speed = 8.0f,
    .damage = damage_,
  });
}
