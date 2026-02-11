#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Graphic/Frame/frame_packet.h"
#include "SceneSetting/active_camera_setting.h"
#include "SceneSetting/active_ui_camera_setting.h"
#include "SceneSetting/background_setting.h"
#include "SceneSetting/light_setting.h"
#include "SceneSetting/shadow_setting.h"
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

  ActiveCameraSetting& GetCameraSetting() {
    return camera_setting_;
  }
  const ActiveCameraSetting& GetCameraSetting() const {
    return camera_setting_;
  }
  ActiveUICameraSetting& GetUICameraSetting() {
    return ui_camera_setting_;
  }
  const ActiveUICameraSetting& GetUICameraSetting() const {
    return ui_camera_setting_;
  }
  BackgroundSetting& GetBackgroundSetting() {
    return background_setting_;
  }
  const BackgroundSetting& GetBackgroundSetting() const {
    return background_setting_;
  }
  LightSetting& GetLightSetting() {
    return light_setting_;
  }
  const LightSetting& GetLightSetting() const {
    return light_setting_;
  }
  ShadowSetting& GetShadowSetting() {
    return shadow_setting_;
  }
  const ShadowSetting& GetShadowSetting() const {
    return shadow_setting_;
  }

 protected:
  GameObject* CreateGameObject(const std::string& name = "GameObject", const TransformComponent::Props& transform = {});
  GameObject* FindGameObject(const std::string& name) const;

 public:
  const std::vector<std::unique_ptr<GameObject>>& GetGameObjects() const {
    return game_objects_;
  }

  void DestroyGameObject(GameObject* obj);
  void DestroyGameObject(const std::string& name);

 private:
  std::vector<std::unique_ptr<GameObject>> game_objects_;
  ActiveCameraSetting camera_setting_;
  ActiveUICameraSetting ui_camera_setting_;
  BackgroundSetting background_setting_;
  LightSetting light_setting_;
  ShadowSetting shadow_setting_;
  bool is_started_ = false;

  GameContext* context_ = nullptr;

  void FlushPendingStarts();
  void CleanupDestroyedObjects();
  void UpdateRootObjects(float dt);
  void FixedUpdateRootObjects(float dt);
  void RenderRootObjects(FramePacket& packet);
  void StartAllObjects();
};
