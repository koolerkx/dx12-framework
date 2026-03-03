#pragma once

#include <cstdint>
#include <vector>

#include "Component/behavior_component.h"
#include "Scenes/city_scene/city_scene_config.h"
#include "Scenes/city_scene/wave_stage_config.h"

class EnemySpawnManagerComponent;

enum class WaveState : uint8_t {
  WaitingInitial,
  Spawning,
  WaveDelay,
};

struct SpawnerProgress {
  int spawner_index;
  int spawned_so_far;
  int target_count;
  float interval_timer;
};

class WaveControllerComponent : public BehaviorComponent<WaveControllerComponent> {
 public:
  struct Props {
    CitySceneConfig::WaveTimingConfig wave_timing;
  };

  using BehaviorComponent::BehaviorComponent;
  WaveControllerComponent(GameObject* owner, const Props& props);

  void OnStart() override;

  int GetCurrentWave() const {
    return current_wave_;
  }
  const CitySceneConfig::WaveTimingConfig& GetWaveTiming() const {
    return props_.wave_timing;
  }

  WaveState GetWaveState() const {
    return wave_state_;
  }

  void SetWaveState(WaveState state) {
    wave_state_ = state;
  }

  float GetTimer() const {
    return timer_;
  }

  void SetTimer(float t) {
    timer_ = t;
  }

  void IncrementWave() {
    ++current_wave_;
  }

  EnemySpawnManagerComponent* GetSpawnManager() const {
    return spawn_manager_;
  }

  WaveStageConfig& GetCurrentConfig() {
    return current_config_;
  }

  std::vector<SpawnerProgress>& GetSpawnerProgress() {
    return spawner_progress_;
  }

 private:
  Props props_;
  EnemySpawnManagerComponent* spawn_manager_ = nullptr;
  WaveStageConfig current_config_;
  int current_wave_ = 0;
  float timer_ = 0.0f;
  WaveState wave_state_ = WaveState::WaitingInitial;
  std::vector<SpawnerProgress> spawner_progress_;
};
