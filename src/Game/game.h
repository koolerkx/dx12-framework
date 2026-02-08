// game.h
#pragma once
#include <memory>

#include "Asset/asset_manager.h"
#include "Graphic/Frame/frame_packet.h"
#include "scene_manager.h"

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

  IScene* GetCurrentScene() const {
    return scene_manager_.GetCurrentScene();
  }

  void OnUpdate(float dt);
  void OnFixedUpdate(float dt);
  void OnRender();

 private:
  GameContext* context_ = nullptr;
  AssetManager asset_manager_;
  std::unique_ptr<DebugDrawer> debug_drawer_;
  SceneManager scene_manager_;
  FramePacket frame_packet_;

  std::atomic<bool> is_shutting_down_{false};
};
