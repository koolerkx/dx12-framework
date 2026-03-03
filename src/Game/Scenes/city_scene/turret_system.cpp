#include "Scenes/city_scene/turret_system.h"

#include "Asset/asset_manager.h"
#include "Component/model_component.h"
#include "Component/transform_component.h"
#include "Framework/Math/Math.h"
#include "Scenes/city_scene/city_scene_config.h"
#include "Scenes/city_scene/enemy_component.h"
#include "Scenes/city_scene/projectile_component.h"
#include "Scenes/city_scene/turret_component.h"
#include "game_context.h"
#include "game_object.h"
#include "scene.h"

namespace cfg = CitySceneConfig;

using Math::Vector3;

TurretSystem::TurretSystem(IScene* scene) : scene_(scene) {
  projectile_model_ = scene_->GetContext()->GetAssetManager().LoadModel(cfg::PATHS.projectile_model, cfg::FBX_UNIT_SCALE);
}

void TurretSystem::Update(float dt) {
  auto* enemy_manager = scene_->FindGameObject("EnemyManager");
  if (!enemy_manager) return;

  const auto& objects = scene_->GetGameObjects();
  for (size_t i = 0; i < objects.size(); ++i) {
    auto* turret = objects[i]->GetComponent<TurretComponent>();
    if (!turret || !turret->IsRunning()) continue;

    auto tower_pos = objects[i]->GetTransform()->GetWorldPosition();
    float range_sq = turret->GetRange() * turret->GetRange();

    GameObject* nearest = nullptr;
    float nearest_dist_sq = range_sq;

    for (auto* child : enemy_manager->GetChildren()) {
      if (child->IsPendingDestroy()) continue;
      if (!child->GetComponent<EnemyComponent>()) continue;

      auto enemy_pos = child->GetTransform()->GetWorldPosition();
      float dist_sq = Vector3::DistanceSquared(tower_pos, enemy_pos);
      if (dist_sq < nearest_dist_sq) {
        nearest_dist_sq = dist_sq;
        nearest = child;
      }
    }

    if (nearest) {
      turret->SetLaserTarget(nearest->GetTransform()->GetWorldPosition());
    } else {
      turret->ClearLaserTarget();
    }

    turret->AdvanceShootTimer(dt);

    if (turret->IsReadyToShoot() && nearest) {
      SpawnProjectile(tower_pos, nearest, turret->GetDamage());
      turret->ResetShootTimer();
    }
  }
}

void TurretSystem::SpawnProjectile(const Vector3& origin, GameObject* target, float damage) {
  static int global_projectile_id = 0;

  auto* projectile = scene_->CreateGameObject("Projectile_" + std::to_string(global_projectile_id++), {.position = origin});
  projectile->SetTransient(true);

  projectile->AddComponent<ModelComponent>(ModelComponent::Props{.model = projectile_model_});

  projectile->AddComponent<ProjectileComponent>(ProjectileComponent::Props{
    .target = target,
    .speed = 8.0f,
    .damage = damage,
  });
}
