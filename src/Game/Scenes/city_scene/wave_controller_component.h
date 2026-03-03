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
  void OnUpdate(float dt) override;

  int GetCurrentWave() const {
    return current_wave_;
  }

 private:
  void StartWave();

  Props props_;
  EnemySpawnManagerComponent* spawn_manager_ = nullptr;
  WaveStageConfig current_config_;
  int current_wave_ = 0;
  float timer_ = 0.0f;
  WaveState wave_state_ = WaveState::WaitingInitial;
  std::vector<SpawnerProgress> spawner_progress_;
};
