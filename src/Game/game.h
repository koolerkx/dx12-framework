// game.h
#pragma once
#include <memory>

#include "Asset/asset_manager.h"
#include "Graphic/Frame/frame_packet.h"
#include "scene.h"

class GameContext;

class Game {
 public:
  Game();
  ~Game();

  void SetContext(GameContext* context) {
    context_ = context;
  }

  void Initialize();
  void Shutdown();

  void OnUpdate(float dt);
  void OnFixedUpdate(float dt);
  void OnRender();

 private:
  GameContext* context_ = nullptr;
  AssetManager asset_manager_;
  std::unique_ptr<DebugDrawer> debug_drawer_;
  std::unique_ptr<IScene> current_scene_;
  FramePacket frame_packet_;

  std::atomic<bool> is_shutting_down_{false};
};
