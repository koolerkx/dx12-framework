#pragma once

#include "Map/map_data.h"
#include "Map/nav_grid.h"
#include "scene.h"

class CityScene : public IScene {
 public:
  void OnEnter(AssetManager& asset_manager) override;
  void OnExit() override;
  void OnDebugDraw(DebugDrawer& drawer) override;

 private:
  void SetupCamera();
  void SetupCameraBounds(const MapData& map_data);
  void SpawnEnemyManager();
  void SpawnBorderWalls(const MapData& map_data);
  void CreateSpawnCubes(const MapData& map_data);

  NavGrid nav_grid_;
};
