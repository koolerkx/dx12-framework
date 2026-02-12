#pragma once

#include "scene.h"

class TestScene : public IScene {
 public:
  void OnEnter(AssetManager& asset_manager) override;
  void OnExit() override;

  void OnPostUpdate(float dt) override;
  void OnDebugDraw(DebugDrawer& drawer) override;

 private:
  GameObject* cube_object_ = nullptr;
  GameObject* cube_object2_ = nullptr;
  GameObject* pivot_cube_ = nullptr;
  GameObject* sphere_object_ = nullptr;

  void SetupCamera();
};
