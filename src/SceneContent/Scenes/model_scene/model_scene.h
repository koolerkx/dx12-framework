#pragma once

#include "Game/scene_key.h"
#include "scene.h"

class ModelScene : public IScene {
 public:
  void OnEnter(AssetManager& asset_manager) override;
  void OnExit() override;
  void OnDebugDraw(DebugDrawer& drawer) override;

 private:
  void SetupCamera();
};

template <>
struct SceneKeyTrait<ModelScene> {
  static constexpr std::string_view NAME = "model";
  static constexpr SceneKey KEY = MakeSceneKey(NAME);
};
