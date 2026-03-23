#pragma once

#include "Game/scene_key.h"
#include "scene.h"

class CubeScene : public IScene {
 public:
  void OnEnter(AssetManager& asset_manager) override;
  void OnExit() override;
  void OnDebugDraw(DebugDrawer& drawer) override;

  void OnPostUpdate(float dt) override;

 private:
  GameObject* cube_ = nullptr;
  float rotation_angle_ = 0.0f;

  void SetupCamera();
};

template <>
struct SceneKeyTrait<CubeScene> {
  static constexpr std::string_view NAME = "cube";
  static constexpr SceneKey KEY = MakeSceneKey(NAME);
};
