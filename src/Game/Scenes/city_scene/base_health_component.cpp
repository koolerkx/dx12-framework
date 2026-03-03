#include "Scenes/city_scene/base_health_component.h"

#include "Component/player_spawn_component.h"
#include "Component/transform_component.h"
#include "Framework/Event/event_bus.hpp"
#include "Framework/Logging/logger.h"
#include "Scenes/city_scene/city_scene_config.h"
#include "Scenes/city_scene/city_scene_events.h"
#include "Scenes/city_scene/explosion_effect.h"
#include "Scripts/camera_shake_controller.h"
#include "Scripts/screen_effect_controller.h"
#include "game_context.h"
#include "game_object.h"
#include "scene.h"
#include "scene_events.h"

BaseHealthComponent::BaseHealthComponent(GameObject* owner, const Props& /*props*/) : BehaviorComponent(owner) {
}

void BaseHealthComponent::OnStart() {
  const CitySceneConfig::PlayerSpawnConfig health_cfg;
  health_ = health_cfg.max_health;
  Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(), "[Health] Initialized with {} health", health_);
  GetContext()->GetEventBus()->Emit(HealthChangedEvent{.health = health_});
}

void BaseHealthComponent::TakeDamage(int amount) {
  if (health_ <= 0) return;
  health_ -= amount;
  Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(), "[Health] -{} health (remaining: {})", amount, health_);

  auto bus = GetContext()->GetEventBus();
  bus->Emit(HealthChangedEvent{.health = health_});
  bus->Emit(EnemyReachedBaseEvent{});

  if (health_ <= 0) {
    GetContext()->GetEventBus()->Emit(BaseDestroyedEvent{});
    SpawnBaseDestroyedEffect();
  }
}

void BaseHealthComponent::SpawnBaseDestroyedEffect() {
  auto* scene = GetOwner()->GetScene();
  if (!scene) return;

  GameObject* spawn_go = nullptr;
  for (const auto& go : scene->GetGameObjects()) {
    if (go->GetComponent<PlayerSpawnComponent>()) {
      spawn_go = go.get();
      break;
    }
  }
  if (!spawn_go) return;

  auto pos = spawn_go->GetTransform()->GetWorldPosition();

  const CitySceneConfig::BaseDestroyedExplosionConfig explosion_cfg;
  CitySceneEffect::SpawnExplosion(scene, pos, CitySceneEffect::FromBaseDestroyedConfig(explosion_cfg), "BaseDestroyed");

  const CitySceneConfig::BaseDestroyedSparksConfig sparks_cfg;
  CitySceneEffect::SpawnExplosionSparks(scene, pos, CitySceneEffect::FromBaseDestroyedSparksConfig(sparks_cfg), "BaseDestroyedSparks");

  const CitySceneConfig::BaseDestroyedScreenEffectConfig fx_cfg;
  auto* camera_go = scene->FindGameObject("MainCamera");
  if (camera_go) {
    if (auto* shake = camera_go->GetComponent<CameraShakeController>()) shake->Trigger(fx_cfg.shake_intensity, fx_cfg.shake_duration);
    if (auto* screen_fx = camera_go->GetComponent<ScreenEffectController>())
      screen_fx->TriggerChromaticAberration(fx_cfg.chromatic_aberration_intensity);
  }

  spawn_go->Destroy();
}
