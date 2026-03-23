#pragma once
#include "Debug/debug_drawer.h"
#include "Framework/Asset/asset_manager.h"
#include "Framework/Event/event_bus.hpp"
#include "Framework/Input/input.h"
#include "Framework/Render/render_service.h"
#include "play_state.h"
#include "scene_defaults.h"

class SceneManager;
class ITextureService;
class IMeshService;
class IFontService;
class IDebugDrawService;
class IShaderRegistration;

class GameContext {
 public:
  GameContext() = default;

  InputSystem* GetInput() const {
    return input_;
  }
  void SetInputSystem(InputSystem* input) {
    input_ = input;
  }

  std::shared_ptr<EventBus> GetEventBus() const {
    return event_bus_;
  }
  void SetEventBus(std::shared_ptr<EventBus> bus) {
    event_bus_ = std::move(bus);
  }

  AssetManager& GetAssetManager() const {
    return *asset_manager_;
  }
  void SetAssetManager(AssetManager* asset_manager) {
    asset_manager_ = asset_manager;
  }

  DebugDrawer* GetDebugDrawer() const {
    return debug_drawer_;
  }
  void SetDebugDrawer(DebugDrawer* debug_drawer) {
    debug_drawer_ = debug_drawer;
  }

  IRenderService* GetRenderService() const {
    return render_service_;
  }
  void SetRenderService(IRenderService* render_service) {
    render_service_ = render_service;
  }

  SceneManager* GetSceneManager() const {
    return scene_manager_;
  }
  void SetSceneManager(SceneManager* scene_manager) {
    scene_manager_ = scene_manager;
  }

  PlayState GetPlayState() const {
    return play_state_ ? *play_state_ : PlayState::Stopped;
  }
  void SetPlayStateSource(PlayState* play_state) {
    play_state_ = play_state;
  }

  const SceneDefaults& GetSceneDefaults() const {
    return scene_defaults_;
  }
  void SetSceneDefaults(const SceneDefaults& defaults) {
    scene_defaults_ = defaults;
  }

  void RequestQuit() {
    quit_requested_ = true;
  }
  bool IsQuitRequested() const {
    return quit_requested_;
  }

  // Service accessors
  ITextureService* GetTextureService() const {
    return texture_service_;
  }
  void SetTextureService(ITextureService* service) {
    texture_service_ = service;
  }

  IMeshService* GetMeshService() const {
    return mesh_service_;
  }
  void SetMeshService(IMeshService* service) {
    mesh_service_ = service;
  }

  IFontService* GetFontService() const {
    return font_service_;
  }
  void SetFontService(IFontService* service) {
    font_service_ = service;
  }

  IDebugDrawService* GetDebugDrawService() const {
    return debug_draw_service_;
  }
  void SetDebugDrawService(IDebugDrawService* service) {
    debug_draw_service_ = service;
  }

  IShaderRegistration* GetShaderRegistration() const {
    return shader_registration_;
  }
  void SetShaderRegistration(IShaderRegistration* service) {
    shader_registration_ = service;
  }

 private:
  InputSystem* input_ = nullptr;
  IRenderService* render_service_ = nullptr;
  std::shared_ptr<EventBus> event_bus_;
  AssetManager* asset_manager_ = nullptr;
  DebugDrawer* debug_drawer_ = nullptr;
  SceneManager* scene_manager_ = nullptr;
  PlayState* play_state_ = nullptr;
  SceneDefaults scene_defaults_;
  bool quit_requested_ = false;

  ITextureService* texture_service_ = nullptr;
  IMeshService* mesh_service_ = nullptr;
  IFontService* font_service_ = nullptr;
  IDebugDrawService* debug_draw_service_ = nullptr;
  IShaderRegistration* shader_registration_ = nullptr;
};
