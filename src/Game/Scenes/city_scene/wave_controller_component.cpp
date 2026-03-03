#include "Scenes/city_scene/wave_controller_component.h"

#include "Framework/Event/event_bus.hpp"
#include "Framework/Logging/logger.h"
#include "Scenes/city_scene/city_scene_events.h"
#include "Scenes/city_scene/enemy_spawn_manager_component.h"
#include "Scenes/city_scene/game_match_component.h"
#include "game_context.h"
#include "game_object.h"
#include "scene.h"

WaveControllerComponent::WaveControllerComponent(GameObject* owner, const Props& props) : BehaviorComponent(owner), props_(props) {
}

void WaveControllerComponent::OnStart() {
  auto* enemy_manager = GetOwner()->GetScene()->FindGameObject("EnemyManager");
  if (enemy_manager) {
    spawn_manager_ = enemy_manager->GetComponent<EnemySpawnManagerComponent>();
  }
}

void WaveControllerComponent::OnUpdate(float dt) {
  auto* match = GetOwner()->GetComponent<GameMatchComponent>();
  if (match && match->IsGameOver()) return;
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
        Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(), "[Wave] Wave {} spawning complete", current_wave_);
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

void WaveControllerComponent::StartWave() {
  current_config_ = WaveStageConfig::Generate(current_wave_, spawn_manager_->GetSpawnerCount());

  Logger::LogFormat(LogLevel::Info,
    LogCategory::Game,
    Logger::Here(),
    "[Wave] Wave {} started: {} enemies, {} spawners, interval {:.2f}s",
    current_wave_,
    current_config_.total_enemy_count,
    current_config_.spawner_assignments.size(),
    current_config_.spawn_interval);

  GetContext()->GetEventBus()->Emit(WaveStartEvent{.wave = current_wave_});

  spawner_progress_.clear();
  for (const auto& assignment : current_config_.spawner_assignments) {
    Logger::LogFormat(LogLevel::Info,
      LogCategory::Game,
      Logger::Here(),
      "[Wave]   Spawner {}: {} enemies",
      assignment.spawner_index,
      assignment.enemy_count);

    spawner_progress_.push_back({
      .spawner_index = assignment.spawner_index,
      .spawned_so_far = 0,
      .target_count = assignment.enemy_count,
      .interval_timer = 0.0f,
    });
  }
}
