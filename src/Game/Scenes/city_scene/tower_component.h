#pragma once

#include <cstdint>
#include <memory>

#include "Component/behavior_component.h"
#include "Framework/Event/event_scope.hpp"

struct ModelData;
class GameObject;

enum class LaserVisibility : uint8_t { AlwaysInRange, HighlightedOnly };

class TowerComponent : public BehaviorComponent<TowerComponent> {
 public:
  struct Props {
    float range = 3.0f;
    float shoot_interval = 1.0f;
    float damage = 1.0f;
    LaserVisibility laser_visibility = LaserVisibility::HighlightedOnly;
  };

  using BehaviorComponent::BehaviorComponent;
  TowerComponent(GameObject* owner, const Props& props)
      : BehaviorComponent(owner),
        range_(props.range),
        shoot_interval_(props.shoot_interval),
        damage_(props.damage),
        laser_visibility_(props.laser_visibility) {
  }

  void OnStart() override;
  void OnUpdate(float dt) override;
  void OnDestroy() override;

  void SetHighlighted(bool highlighted) {
    highlighted_ = highlighted;
  }

 private:
  bool ShouldShowLaser() const;
  GameObject* FindNearestEnemy() const;
  void SpawnProjectile(GameObject* target);
  void UpdateLaser(GameObject* target);
  void DestroyLaser();

  float range_ = 3.0f;
  float shoot_interval_ = 1.0f;
  float damage_ = 1.0f;
  float shoot_timer_ = 0.0f;
  LaserVisibility laser_visibility_ = LaserVisibility::HighlightedOnly;
  bool highlighted_ = false;
  bool is_running_ = true;
  EventScope event_scope_;
  GameObject* enemy_manager_ = nullptr;
  std::shared_ptr<ModelData> projectile_model_;
  GameObject* laser_go_ = nullptr;
};
