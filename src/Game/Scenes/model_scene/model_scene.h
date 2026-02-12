#pragma once

#include "scene.h"

class ModelScene : public IScene {
 public:
  void OnEnter(AssetManager& asset_manager) override;
  void OnExit() override;
  void OnDebugDraw(DebugDrawer& drawer) override;
  void OnPostUpdate(float dt) override;

 private:
  void SetupCamera();
};
