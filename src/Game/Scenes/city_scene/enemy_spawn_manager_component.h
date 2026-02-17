#pragma once

#include <memory>
#include <vector>

#include "Component/behavior_component.h"
#include "Framework/Math/Math.h"

class NavGrid;
class GameObject;
struct ModelData;

class EnemySpawnManagerComponent : public BehaviorComponent<EnemySpawnManagerComponent> {
 public:
  struct Props {
    NavGrid* nav = nullptr;
  };

  EnemySpawnManagerComponent(GameObject* owner, const Props& props);

  void OnStart() override;
  void OnReset() override;

  void SpawnEnemy(int spawner_index);
  int GetSpawnerCount() const;

 private:
  void SpawnEnemyAt(GameObject* spawner);

  NavGrid* nav_ = nullptr;
  std::vector<GameObject*> enemy_spawners_;
  Math::Vector2 player_spawn_xz_ = {};
  std::shared_ptr<ModelData> enemy_model_;
  int enemy_counter_ = 0;
};
