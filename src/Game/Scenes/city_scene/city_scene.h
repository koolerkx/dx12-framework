#pragma once

#include <memory>

#include "Map/map_data.h"
#include "Map/nav_grid.h"
#include "SceneSetting/scene_transition_overlay.h"
#include "scene.h"

class WaveSystem;

class CityScene : public IScene {
 public:
  CityScene();
  ~CityScene() override;

  void OnEnter(AssetManager& asset_manager) override;
  void OnExit() override;
  void OnPreUpdate(float dt) override;
  void OnRender(FramePacket& packet) override;
  void OnDebugDraw(DebugDrawer& drawer) override;

 private:
  void SetupCamera();
  void SetupCameraBounds(const MapData& map_data);
  void SpawnEnemyManager();
  void SpawnBorderWalls(const MapData& map_data);
  void CreateSpawnCubes(const MapData& map_data);

  NavGrid nav_grid_;
  SceneTransitionOverlay transition_overlay_;
  std::unique_ptr<WaveSystem> wave_system_;
};
