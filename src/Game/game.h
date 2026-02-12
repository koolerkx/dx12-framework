// game.h
#pragma once
#include <memory>

#include "Asset/asset_manager.h"
#include "Graphic/Frame/frame_packet.h"
#include "play_state.h"
#include "scene_manager.h"

class GameContext;

class Game {
 public:
  Game();
  ~Game();

  struct Props {
    GameContext* context = nullptr;
    bool auto_play = false;
  };

  void Initialize(const Props& props);
  void Shutdown();

  IScene* GetCurrentScene() const {
    return scene_manager_.GetCurrentScene();
  }

  PlayState GetPlayState() const { return play_state_; }
  void Play();
  void Pause();
  void Stop();

  void OnUpdate(float dt);
  void OnFixedUpdate(float dt);
  void OnRender();

 private:
  PlayState play_state_ = PlayState::Stopped;
  GameContext* context_ = nullptr;
  AssetManager asset_manager_;
  std::unique_ptr<DebugDrawer> debug_drawer_;
  SceneManager scene_manager_;
  FramePacket frame_packet_;

  std::atomic<bool> is_shutting_down_{false};
};
