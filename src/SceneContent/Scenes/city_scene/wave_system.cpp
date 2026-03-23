#include "Scenes/city_scene/wave_system.h"

#include "Framework/Event/event_bus.hpp"
#include "Framework/Logging/logger.h"
#include "Scenes/city_scene/city_scene_events.h"
#include "Scenes/city_scene/enemy_spawn_manager_component.h"
#include "Scenes/city_scene/game_match_component.h"
#include "Scenes/city_scene/wave_controller_component.h"
#include "game_context.h"
#include "game_object.h"
#include "scene.h"

WaveSystem::WaveSystem(IScene* scene) : scene_(scene) {
}

void WaveSystem::Update(float dt) {
  auto* player = scene_->FindGameObject("Player");
  if (!player) return;

  auto* match = player->GetComponent<GameMatchComponent>();
  if (match && match->IsGameOver()) return;

  auto* wave = player->GetComponent<WaveControllerComponent>();
  if (!wave) return;

  auto* spawn_manager = wave->GetSpawnManager();
  if (!spawn_manager) return;

  auto* event_bus = scene_->GetContext()->GetEventBus().get();

  switch (wave->GetWaveState()) {
    case WaveState::WaitingInitial: {
      wave->SetTimer(wave->GetTimer() + dt);
      float remaining = wave->GetWaveTiming().initial_delay - wave->GetTimer();
      if (remaining > 0.0f) {
        event_bus->Emit(WaveCountdownEvent{.seconds_remaining = remaining});
      } else {
        wave->SetTimer(0.0f);
        StartWave();
        wave->SetWaveState(WaveState::Spawning);
      }
      break;
    }

    case WaveState::Spawning: {
      bool all_done = true;
      auto& config = wave->GetCurrentConfig();
      for (auto& sp : wave->GetSpawnerProgress()) {
        if (sp.spawned_so_far >= sp.target_count) continue;
        all_done = false;

        sp.interval_timer += dt;
        if (sp.interval_timer >= config.spawn_interval) {
          sp.interval_timer -= config.spawn_interval;
          spawn_manager->SpawnEnemy(sp.spawner_index, wave->GetCurrentWave());
          ++sp.spawned_so_far;
        }
      }

      if (all_done) {
        Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(), "[Wave] Wave {} spawning complete", wave->GetCurrentWave());
        wave->SetTimer(0.0f);
        wave->SetWaveState(WaveState::WaveDelay);
      }
      break;
    }

    case WaveState::WaveDelay: {
      wave->SetTimer(wave->GetTimer() + dt);
      float remaining = wave->GetWaveTiming().wave_delay - wave->GetTimer();
      if (remaining > 0.0f) {
        event_bus->Emit(WaveCountdownEvent{.seconds_remaining = remaining});
      } else {
        wave->SetTimer(0.0f);
        wave->IncrementWave();
        StartWave();
        wave->SetWaveState(WaveState::Spawning);
      }
      break;
    }
  }
}

void WaveSystem::StartWave() {
  auto* player = scene_->FindGameObject("Player");
  if (!player) return;

  auto* wave = player->GetComponent<WaveControllerComponent>();
  if (!wave) return;

  auto* spawn_manager = wave->GetSpawnManager();
  if (!spawn_manager) return;

  auto& config = wave->GetCurrentConfig();
  config = WaveStageConfig::Generate(wave->GetCurrentWave(), spawn_manager->GetSpawnerCount());

  Logger::LogFormat(LogLevel::Info,
    LogCategory::Game,
    Logger::Here(),
    "[Wave] Wave {} started: {} enemies, {} spawners, interval {:.2f}s",
    wave->GetCurrentWave(),
    config.total_enemy_count,
    config.spawner_assignments.size(),
    config.spawn_interval);

  scene_->GetContext()->GetEventBus()->Emit(WaveStartEvent{.wave = wave->GetCurrentWave()});

  auto& spawner_progress = wave->GetSpawnerProgress();
  spawner_progress.clear();
  for (const auto& assignment : config.spawner_assignments) {
    Logger::LogFormat(LogLevel::Info,
      LogCategory::Game,
      Logger::Here(),
      "[Wave]   Spawner {}: {} enemies",
      assignment.spawner_index,
      assignment.enemy_count);

    spawner_progress.push_back({
      .spawner_index = assignment.spawner_index,
      .spawned_so_far = 0,
      .target_count = assignment.enemy_count,
      .interval_timer = 0.0f,
    });
  }
}
