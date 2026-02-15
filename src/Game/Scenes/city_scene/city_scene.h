#pragma once

#include "scene.h"

class CityScene : public IScene {
 public:
  void OnEnter(AssetManager& asset_manager) override;
  void OnExit() override;
  void OnDebugDraw(DebugDrawer& drawer) override;

 private:
  void SetupCamera();
  void CreateScanlineCube();
};
