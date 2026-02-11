#pragma once

#include "Asset/asset_handle.h"
#include "scene.h"

struct Texture;

class TestScene : public IScene {
 public:
  void OnEnter(AssetManager& asset_manager) override;
  void OnExit() override;

  void OnPostUpdate(float dt) override;
  void OnDebugDraw(DebugDrawer& drawer) override;

 private:
  AssetHandle<Texture> texture_background_;
  AssetHandle<Texture> texture_character_;
  AssetHandle<Texture> texture_ao_;
  AssetHandle<Texture> texture_additive_;
  GameObject* cube_object_ = nullptr;
  GameObject* cube_object2_ = nullptr;
  GameObject* pivot_cube_ = nullptr;
  GameObject* sphere_object_ = nullptr;

  void SetupCamera();
};
