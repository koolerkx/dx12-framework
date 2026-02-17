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

using Math::Quaternion;
using Math::Vector3;

void TowerComponent::OnStart() {
  enemy_manager_ = GetOwner()->GetScene()->FindGameObject("EnemyManager");
  projectile_model_ = GetContext()->GetAssetManager().LoadModel(cfg::PATHS.projectile_model, cfg::FBX_UNIT_SCALE);
}

void TowerComponent::OnUpdate(float dt) {
  if (!enemy_manager_) return;

  auto* target = FindNearestEnemy();

  if (target && ShouldShowLaser()) {
    UpdateLaser(target);
  } else {
    DestroyLaser();
  }

  shoot_timer_ += dt;
  if (shoot_timer_ < shoot_interval_) return;

  if (!target) return;

  SpawnProjectile(target);
  shoot_timer_ = 0.0f;
}

bool TowerComponent::ShouldShowLaser() const {
  return laser_visibility_ == LaserVisibility::AlwaysInRange || highlighted_;
}

void TowerComponent::OnDestroy() {
  DestroyLaser();
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

void TowerComponent::UpdateLaser(GameObject* target) {
  constexpr float BEAM_THICKNESS = 0.05f;

  auto tower_pos = GetOwner()->GetTransform()->GetWorldPosition();
  auto enemy_pos = target->GetTransform()->GetWorldPosition();

  auto direction = enemy_pos - tower_pos;
  float distance = direction.Length();
  if (distance < 0.001f) {
    DestroyLaser();
    return;
  }

  direction = direction / distance;

  if (!laser_go_) {
    static int laser_id = 0;
    auto* scene = GetOwner()->GetScene();
    laser_go_ = scene->CreateGameObject("Laser_" + std::to_string(laser_id++), {});
    laser_go_->SetTransient(true);

    auto* renderer = laser_go_->AddComponent<MeshRenderer>(MeshRenderer::Props{
      .mesh_type = DefaultMesh::Cylinder,
    });
    renderer->SetShaderWithParams<Graphics::LaserBeamShader>(Graphics::LaserBeamShader::Params{
      .laser_r = 1.0f,
      .laser_g = 0.2f,
      .laser_b = 0.2f,
      .emissive_intensity = 10.0f,
      .pulse_speed = 15.0f,
      .pulse_frequency = 20.0f,
      .beam_width = 2.0f,
      .end_fade_ratio = 0.4f,
      .end_fade_power = 3.0f,
    });
  }

  auto midpoint = (tower_pos + enemy_pos) * 0.5f;
  auto rotation = Quaternion::FromToRotation(Vector3::Up, direction);

  auto* transform = laser_go_->GetTransform();
  transform->SetPosition(midpoint);
  transform->SetRotation(rotation);
  transform->SetScale({BEAM_THICKNESS, distance, BEAM_THICKNESS});
}

void TowerComponent::DestroyLaser() {
  if (laser_go_) {
    laser_go_->Destroy();
    laser_go_ = nullptr;
  }
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
