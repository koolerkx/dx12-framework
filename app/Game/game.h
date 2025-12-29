// game.h
#pragma once
#include <memory>

#include "Asset/asset_manager.h"
#include "Graphic/frame_packet.h"
#include "Graphic/graphic.h"
#include "scene.h"

class Game {
 public:
  explicit Game(Graphic& graphic);
  ~Game();

  void Initialize();
  void Shutdown();

  void OnUpdate(float dt);
  void OnFixedUpdate(float dt);
  void OnRender();

 private:
  Graphic& graphic_;
  AssetManager asset_manager_;
  std::unique_ptr<IScene> current_scene_;
  FramePacket frame_packet_;

  std::atomic<bool> is_shutting_down_{false};
};
