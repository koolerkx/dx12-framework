#include "projectile_component.h"

#include <cmath>

#include "Component/transform_component.h"
#include "Framework/Math/Math.h"
#include "Scenes/city_scene/enemy_component.h"
#include "game_object.h"

using Math::Vector3;

void ProjectileComponent::OnUpdate(float dt) {
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
    auto* enemy = target_->GetComponent<EnemyComponent>();
    if (enemy) {
      enemy->TakeDamage(damage_);
    }
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
