#pragma once

#include <vector>

#include "Component/behavior_component.h"
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

class GameStateManagerComponent : public BehaviorComponent<GameStateManagerComponent> {
 public:
  struct Props {
    float initial_delay = 3.0f;
    float wave_delay = 5.0f;
  };

  using BehaviorComponent::BehaviorComponent;
  GameStateManagerComponent(GameObject* owner, const Props& props);

  void OnStart() override;
  void OnUpdate(float dt) override;

  void AddGold(int amount);
  bool TrySpendGold(int amount);
  int GetGold() const { return gold_; }

  void TakeDamage(int amount = 1);
  int GetHealth() const { return health_; }

  int GetCurrentWave() const { return current_wave_; }

 private:
  void StartWave();
  void SpawnBaseDestroyedEffect();

  Props props_;
  int gold_ = 0;
  int health_ = 0;

  EnemySpawnManagerComponent* spawn_manager_ = nullptr;
  WaveStageConfig current_config_;
  int current_wave_ = 0;
  float timer_ = 0.0f;
  WaveState wave_state_ = WaveState::WaitingInitial;
  std::vector<SpawnerProgress> spawner_progress_;
};
