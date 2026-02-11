#pragma once

#include "Asset/asset_handle.h"
#include "Frame/frame_packet.h"
#include "scene.h"

struct Texture;

class CubeScene : public IScene {
 public:
  void OnEnter(AssetManager& asset_manager) override;
  void OnExit() override;
  void OnDebugDraw(DebugDrawer& drawer) override;

  void OnPostUpdate(float dt) override;

 private:
  AssetHandle<Texture> texture_;
  AssetHandle<Texture> particle_texture_;
  GameObject* cube_ = nullptr;
  float rotation_angle_ = 0.0f;

  void SetupCamera();
};
