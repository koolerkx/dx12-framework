/**
 * @file game.h
 * @brief Top-level game orchestrator owning SceneManager, AssetManager, and DebugDrawer.
 */
#pragma once
#include <functional>
#include <memory>

#include "Framework/Asset/asset_manager.h"
#include "Framework/Render/frame_packet.h"
#include "play_state.h"
#include "scene_defaults.h"
#include "scene_key.h"
#include "scene_manager.h"

class GameContext;

class Game {
 public:
  using SceneRegistrar = std::function<void(SceneManager&)>;

  Game();
  ~Game();

  struct Props {
    GameContext* context = nullptr;
    bool auto_play = false;
    bool debug_draw_enabled = true;
    SceneDefaults scene_defaults;
  };

  void Initialize(const Props& props);
  void SetSceneRegistrar(SceneRegistrar registrar);
  void LoadInitialScene(SceneKey key);
  void Shutdown();

  IScene* GetCurrentScene() const {
    return scene_manager_.GetCurrentScene();
  }

  PlayState GetPlayState() const {
    return play_state_;
  }
  void Play();
  void Pause();
  void Stop();

  void OnUpdate(float dt);
  void OnFixedUpdate(float dt);
  void CollectRenderData(FramePacket& packet);

  float GetElapsedTime() const {
    return elapsed_time_;
  }

 private:
  PlayState play_state_ = PlayState::Stopped;
  GameContext* context_ = nullptr;
  AssetManager asset_manager_;
  std::unique_ptr<DebugDrawer> debug_drawer_;
  SceneManager scene_manager_;
  float elapsed_time_ = 0.0f;

  SceneRegistrar scene_registrar_;
  std::atomic<bool> is_shutting_down_{false};
};
