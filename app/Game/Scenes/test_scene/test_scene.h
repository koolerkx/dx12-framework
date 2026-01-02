#pragma once
#include "Asset/asset_handle.h"
#include "scene.h"

struct Texture;

class TestScene : public IScene {
 public:
  void OnEnter(AssetManager& asset_manager) override;
  void OnExit() override;

  void OnPostUpdate(float dt) override;
  void OnRender(FramePacket& packet) override;

 private:
  AssetHandle<Texture> texture_background_;
  AssetHandle<Texture> texture_character_;
  AssetHandle<Texture> texture_ao_;
  AssetHandle<Texture> texture_additive_;
  GameObject* character_object_ = nullptr;
  GameObject* character_object2_ = nullptr;
  GameObject* camera_object_ = nullptr;
  GameObject* cube_object_ = nullptr;
  GameObject* cube_object2_ = nullptr;
  GameObject* terrain_plane_ = nullptr;
  GameObject* text_obj_ = nullptr;
  GameObject* text_obj2_ = nullptr;
  GameObject* animated_bg_object_ = nullptr;
  GameObject* additive_ = nullptr;

  void SetupCamera();
};
