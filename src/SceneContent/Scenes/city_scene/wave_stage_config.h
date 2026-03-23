#pragma once

#include <algorithm>
#include <vector>

#include "Framework/Logging/logger.h"

struct SpawnerAssignment {
  int spawner_index;
  int enemy_count;
};

struct WaveStageConfig {
  int wave_index;
  int total_enemy_count;
  float spawn_interval;
  std::vector<SpawnerAssignment> spawner_assignments;

  static WaveStageConfig Generate(int wave_index, int total_spawner_count) {
    WaveStageConfig config;
    config.wave_index = wave_index;
    config.total_enemy_count = 6 + wave_index * 2;
    config.spawn_interval = (std::max)(0.4f, 1.0f - wave_index * 0.05f);

    int active_count = (std::min)(2 + wave_index / 2, total_spawner_count);
    int start_index = wave_index % total_spawner_count;

    std::vector<int> active_spawners;
    for (int i = 0; i < active_count; ++i) {
      active_spawners.push_back((start_index + i) % total_spawner_count);
    }

    bool even_wave = (wave_index % 2 == 0);
    float main_ratio = even_wave ? 0.6f : 0.4f;

    int main_count = static_cast<int>(config.total_enemy_count * main_ratio);
    int remaining = config.total_enemy_count - main_count;

    config.spawner_assignments.push_back({active_spawners[0], main_count});

    int rest_count = static_cast<int>(active_spawners.size()) - 1;
    if (rest_count > 0) {
      int per_spawner = remaining / rest_count;
      int leftover = remaining % rest_count;
      for (int i = 1; i < static_cast<int>(active_spawners.size()); ++i) {
        int count = per_spawner + (i <= leftover ? 1 : 0);
        config.spawner_assignments.push_back({active_spawners[i], count});
      }
    } else {
      config.spawner_assignments[0].enemy_count = config.total_enemy_count;
    }

    Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(), "[Wave] Generated config for wave {}", wave_index);

    return config;
  }
};
