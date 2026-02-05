#pragma once
#include <memory>
#include <vector>

#include "Component/camera_component.h"
#include "Graphic/Frame/frame_packet.h"
#include "game_context.h"
#include "game_object.h"

class AssetManager;
class DebugDrawer;

class IScene {
 public:
  IScene();
  virtual ~IScene();

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

  // Override this to add custom rendering (e.g., debug visualizations)
  virtual void OnRender(FramePacket& /*packet*/) {
  }

  // System Update Functions, this update game objects and components
  void Enter(AssetManager& asset_manager);
  void Exit();
  void Update(float dt);
  void FixedUpdate(float dt);
  void Render(FramePacket& packet);

  void SetContext(GameContext* context) {
    context_ = context;
  }
  GameContext* GetContext() const {
    return context_;
  }

  void SetActiveCamera(CameraComponent* camera) {
    active_camera_ = camera;
  }

  CameraComponent* GetActiveCamera() const {
    return active_camera_;
  }

 protected:
  GameObject* CreateGameObject(const std::string& name = "GameObject");

 public:
  void DestroyGameObject(GameObject* obj);

 private:
  std::vector<std::unique_ptr<GameObject>> game_objects_;
  CameraComponent* active_camera_ = nullptr;
  bool is_started_ = false;

  GameContext* context_ = nullptr;

  void FlushPendingStarts();
  void CleanupDestroyedObjects();
  void UpdateRootObjects(float dt);
  void FixedUpdateRootObjects(float dt);
  void RenderRootObjects(FramePacket& packet);
  void StartAllObjects();
};
