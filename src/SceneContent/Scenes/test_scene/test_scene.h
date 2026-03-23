#pragma once

#include "Game/scene_key.h"
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

template <>
struct SceneKeyTrait<TestScene> {
  static constexpr std::string_view NAME = "test";
  static constexpr SceneKey KEY = MakeSceneKey(NAME);
};
