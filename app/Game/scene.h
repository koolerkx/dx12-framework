#pragma once
#include <memory>
#include <vector>

#include "Graphic/frame_packet.h"
#include "game_object.h"

class AssetManager;

class IScene {
 public:
  virtual ~IScene() = default;

  virtual void OnEnter(AssetManager& asset_manager) = 0;
  virtual void OnExit() = 0;

  // Scene Global Update Functions
  virtual void OnPreUpdate(float /*dt*/) {
  }
  virtual void OnPostUpdate(float /*dt*/) {
  }
  virtual void OnPreFixedUpdate(float /*dt*/) {
  }
  virtual void OnPostFixedUpdate(float /*dt*/) {
  }

  // System Update Functions, this update game objects and components
  void Enter(AssetManager& asset_manager);
  void Exit();
  void Update(float dt);
  void FixedUpdate(float dt);
  void Render(FramePacket& packet);

 protected:
  GameObject* CreateGameObject(const std::string& name = "GameObject");

 private:
  std::vector<std::unique_ptr<GameObject>> game_objects_;
  bool is_started_ = false;

  void UpdateRootObjects(float dt);
  void FixedUpdateRootObjects(float dt);
  void RenderRootObjects(FramePacket& packet);
  void StartAllObjects();
};
