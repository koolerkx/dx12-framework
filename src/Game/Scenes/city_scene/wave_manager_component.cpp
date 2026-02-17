#include "Scenes/city_scene/wave_manager_component.h"

#include "Framework/Logging/logger.h"
#include "Scenes/city_scene/enemy_spawn_manager_component.h"
#include "game_object.h"

WaveManagerComponent::WaveManagerComponent(GameObject* owner, const Props& props) : BehaviorComponent(owner), props_(props) {
}

void WaveManagerComponent::OnInit() {
  spawn_manager_ = GetOwner()->GetComponent<EnemySpawnManagerComponent>();
}

void WaveManagerComponent::OnUpdate(float dt) {
  switch (state_) {
    case WaveState::WaitingInitial: {
      timer_ += dt;
      if (timer_ >= props_.initial_delay) {
        timer_ = 0.0f;
        StartWave();
        state_ = WaveState::Spawning;
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
        state_ = WaveState::WaveDelay;
      }
      break;
    }

    case WaveState::WaveDelay: {
      timer_ += dt;
      if (timer_ >= props_.wave_delay) {
        timer_ = 0.0f;
        ++current_wave_;
        StartWave();
        state_ = WaveState::Spawning;
      }
      break;
    }
  }
}

void WaveManagerComponent::StartWave() {
  current_config_ = WaveStageConfig::Generate(current_wave_, spawn_manager_->GetSpawnerCount());

  Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(),
    "[Wave] Wave {} started: {} enemies, {} spawners, interval {:.2f}s",
    current_wave_, current_config_.total_enemy_count,
    current_config_.spawner_assignments.size(), current_config_.spawn_interval);

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
