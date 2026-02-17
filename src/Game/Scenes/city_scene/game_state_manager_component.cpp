#include "Scenes/city_scene/game_state_manager_component.h"

#include "Component/player_spawn_component.h"
#include "Component/transform_component.h"
#include "Framework/Event/event_bus.hpp"
#include "Framework/Logging/logger.h"
#include "game_context.h"
#include "scene_events.h"
#include "Scenes/city_scene/city_scene_config.h"
#include "Scenes/city_scene/city_scene_events.h"
#include "Scenes/city_scene/enemy_spawn_manager_component.h"
#include "Scenes/city_scene/explosion_effect.h"
#include "Scripts/camera_shake_controller.h"
#include "Scripts/screen_effect_controller.h"
#include "game_object.h"
#include "scene.h"

GameStateManagerComponent::GameStateManagerComponent(GameObject* owner, const Props& props)
    : BehaviorComponent(owner), props_(props) {
}

void GameStateManagerComponent::OnStart() {
  const CitySceneConfig::GoldConfig gold_cfg;
  gold_ = gold_cfg.initial_gold;
  Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(),
    "[Gold] Initialized with {} gold", gold_);

  const CitySceneConfig::PlayerSpawnConfig health_cfg;
  health_ = health_cfg.max_health;
  Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(),
    "[Health] Initialized with {} health", health_);

  auto* enemy_manager = GetOwner()->GetScene()->FindGameObject("EnemyManager");
  if (enemy_manager) {
    spawn_manager_ = enemy_manager->GetComponent<EnemySpawnManagerComponent>();
  }

  auto bus = GetContext()->GetEventBus();
  bus->Emit(GoldChangedEvent{.gold = gold_});
  bus->Emit(HealthChangedEvent{.health = health_});
}

void GameStateManagerComponent::OnUpdate(float dt) {
  if (IsGameOver()) return;
  if (!spawn_manager_) return;

  switch (wave_state_) {
    case WaveState::WaitingInitial: {
      timer_ += dt;
      float remaining = props_.wave_timing.initial_delay - timer_;
      if (remaining > 0.0f) {
        GetContext()->GetEventBus()->Emit(WaveCountdownEvent{.seconds_remaining = remaining});
      } else {
        timer_ = 0.0f;
        StartWave();
        wave_state_ = WaveState::Spawning;
      }
      break;
    }

    case WaveState::Spawning: {
      bool all_done = true;
      for (auto& sp : spawner_progress_) {
        if (sp.spawned_so_far >= sp.target_count) continue;
        all_done = false;

        sp.interval_timer += dt;
        if (sp.interval_timer >= current_config_.spawn_interval) {
          sp.interval_timer -= current_config_.spawn_interval;
          spawn_manager_->SpawnEnemy(sp.spawner_index, current_wave_);
          ++sp.spawned_so_far;
        }
      }

      if (all_done) {
        Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(),
          "[Wave] Wave {} spawning complete", current_wave_);
        timer_ = 0.0f;
        wave_state_ = WaveState::WaveDelay;
      }
      break;
    }

    case WaveState::WaveDelay: {
      timer_ += dt;
      float remaining = props_.wave_timing.wave_delay - timer_;
      if (remaining > 0.0f) {
        GetContext()->GetEventBus()->Emit(WaveCountdownEvent{.seconds_remaining = remaining});
      } else {
        timer_ = 0.0f;
        ++current_wave_;
        StartWave();
        wave_state_ = WaveState::Spawning;
      }
      break;
    }
  }
}

void GameStateManagerComponent::AddGold(int amount) {
  gold_ += amount;
  Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(),
    "[Gold] +{} gold (total: {})", amount, gold_);
  GetContext()->GetEventBus()->Emit(GoldChangedEvent{.gold = gold_});
}

bool GameStateManagerComponent::TrySpendGold(int amount) {
  if (gold_ < amount) {
    Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(),
      "[Gold] Not enough gold: need {}, have {}", amount, gold_);
    GetContext()->GetEventBus()->Emit(InsufficientGoldEvent{.required = amount, .available = gold_});
    return false;
  }
  gold_ -= amount;
  Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(),
    "[Gold] -{} gold (total: {})", amount, gold_);
  GetContext()->GetEventBus()->Emit(GoldChangedEvent{.gold = gold_});
  return true;
}

void GameStateManagerComponent::TakeDamage(int amount) {
  if (health_ <= 0) return;
  health_ -= amount;
  Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(),
    "[Health] -{} health (remaining: {})", amount, health_);

  auto bus = GetContext()->GetEventBus();
  bus->Emit(HealthChangedEvent{.health = health_});
  bus->Emit(EnemyReachedBaseEvent{});

  if (health_ <= 0) {
    game_state_ = GameState::GameOver;
    Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(),
      "[GameOver] Player base destroyed! Wave: {}, Kills: {}", current_wave_, kill_count_);
    GetContext()->GetEventBus()->Emit(GameOverEvent{.wave = current_wave_, .kill_count = kill_count_});
    SpawnBaseDestroyedEffect();
  }
}

void GameStateManagerComponent::SpawnBaseDestroyedEffect() {
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
  CitySceneEffect::SpawnExplosion(scene, pos,
    CitySceneEffect::FromBaseDestroyedConfig(explosion_cfg), "BaseDestroyed");

  const CitySceneConfig::BaseDestroyedSparksConfig sparks_cfg;
  CitySceneEffect::SpawnExplosionSparks(scene, pos,
    CitySceneEffect::FromBaseDestroyedSparksConfig(sparks_cfg), "BaseDestroyedSparks");

  const CitySceneConfig::BaseDestroyedScreenEffectConfig fx_cfg;
  auto* camera_go = scene->FindGameObject("MainCamera");
  if (camera_go) {
    if (auto* shake = camera_go->GetComponent<CameraShakeController>())
      shake->Trigger(fx_cfg.shake_intensity, fx_cfg.shake_duration);
    if (auto* screen_fx = camera_go->GetComponent<ScreenEffectController>())
      screen_fx->TriggerChromaticAberration(fx_cfg.chromatic_aberration_intensity);
  }

  spawn_go->Destroy();
}

void GameStateManagerComponent::IncrementKillCount() {
  ++kill_count_;
}

void GameStateManagerComponent::StartWave() {
  current_config_ = WaveStageConfig::Generate(current_wave_, spawn_manager_->GetSpawnerCount());

  Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(),
    "[Wave] Wave {} started: {} enemies, {} spawners, interval {:.2f}s",
    current_wave_, current_config_.total_enemy_count,
    current_config_.spawner_assignments.size(), current_config_.spawn_interval);

  GetContext()->GetEventBus()->Emit(WaveStartEvent{.wave = current_wave_});

  spawner_progress_.clear();
  for (const auto& assignment : current_config_.spawner_assignments) {
    Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(),
      "[Wave]   Spawner {}: {} enemies", assignment.spawner_index, assignment.enemy_count);

    spawner_progress_.push_back({
      .spawner_index = assignment.spawner_index,
      .spawned_so_far = 0,
      .target_count = assignment.enemy_count,
      .interval_timer = 0.0f,
    });
  }
}
