#pragma once

#include "Asset/asset_handle.h"
#include "scene.h"

struct Texture;

class CubeScene : public IScene {
 public:
  void OnEnter(AssetManager& asset_manager) override;
  void OnExit() override;

  void OnPostUpdate(float dt) override;

 private:
  AssetHandle<Texture> texture_;
  GameObject* cube_ = nullptr;
  float rotation_angle_ = 0.0f;

  void SetupCamera();
};
