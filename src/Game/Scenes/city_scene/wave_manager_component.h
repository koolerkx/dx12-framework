#pragma once

#include <vector>

#include "Component/behavior_component.h"
#include "Scenes/city_scene/wave_stage_config.h"

class EnemySpawnManagerComponent;

enum class WaveState {
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

class WaveManagerComponent : public BehaviorComponent<WaveManagerComponent> {
 public:
  struct Props {
    float initial_delay = 3.0f;
    float wave_delay = 5.0f;
  };

  WaveManagerComponent(GameObject* owner, const Props& props);

  void OnInit() override;
  void OnUpdate(float dt) override;

 private:
  void StartWave();

  Props props_;
  EnemySpawnManagerComponent* spawn_manager_ = nullptr;
  WaveStageConfig current_config_;
  int current_wave_ = 0;
  float timer_ = 0.0f;
  WaveState state_ = WaveState::WaitingInitial;
  std::vector<SpawnerProgress> spawner_progress_;
};
