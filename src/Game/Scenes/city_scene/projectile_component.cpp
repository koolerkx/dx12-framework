#include "projectile_component.h"

#include <cmath>

#include "Component/transform_component.h"
#include "Framework/Math/Math.h"
#include "Scenes/city_scene/explosion_effect.h"
#include "Scenes/city_scene/floating_text_effect.h"
#include "Scenes/city_scene/health_component.h"
#include "game_context.h"
#include "game_object.h"
#include "scene_events.h"


using Math::Vector3;

void ProjectileComponent::OnStart() {
  auto* bus = GetContext()->GetEventBus().get();
  event_scope_.Subscribe<GameOverEvent>(*bus, [this](const GameOverEvent&) { is_running_ = false; });
}

void ProjectileComponent::OnUpdate(float dt) {
  if (!is_running_) return;

  life_time_ -= dt;
  if (life_time_ <= 0.0f) {
    GetOwner()->Destroy();
    return;
  }

  if (!target_ || target_->IsPendingDestroy()) {
    GetOwner()->Destroy();
    return;
  }

  auto* transform = GetOwner()->GetTransform();
  auto pos = transform->GetWorldPosition();
  auto target_pos = target_->GetTransform()->GetWorldPosition();

  Vector3 to_target = target_pos - pos;
  float distance = to_target.Length();
  float step = speed_ * dt;

  if (distance < hit_radius_ || step >= distance) {
    auto* health = target_->GetComponent<HealthComponent>();
    if (health) {
      health->TakeDamage(damage_);
      const CitySceneConfig::FloatingTextConfig txt_cfg;
      CitySceneEffect::SpawnDamageText(GetOwner()->GetScene(), target_pos + Vector3(0, txt_cfg.y_offset, 0), damage_);
    }
    SpawnBulletHitExplosion(target_pos);
    GetOwner()->Destroy();
    return;
  }

  Vector3 direction = to_target / distance;
  Vector3 new_pos = pos + direction * step;
  transform->SetPosition(new_pos);

  float yaw_rad = std::atan2(direction.x, direction.z);
  float yaw_deg = yaw_rad * (180.0f / DirectX::XM_PI) + 180.0f;
  transform->SetRotationEulerDegree(0.0f, yaw_deg, 0.0f);
}

void ProjectileComponent::SpawnBulletHitExplosion(const Vector3& hit_position) {
  auto* scene = GetOwner()->GetScene();
  if (!scene) return;
  const CitySceneConfig::BulletHitConfig cfg;
  CitySceneEffect::SpawnExplosion(scene, hit_position, CitySceneEffect::FromBulletHitConfig(cfg), "BulletHit");

  const CitySceneConfig::BulletHitSparksConfig sparks_cfg;
  CitySceneEffect::SpawnExplosionSparks(scene, hit_position, CitySceneEffect::FromBulletHitSparksConfig(sparks_cfg), "BulletHitSparks");
}
