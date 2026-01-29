#pragma once
#include "Asset/asset_manager.h"
#include "Debug/debug_drawer.h"
#include "Framework/Event/event_system.h"
#include "Framework/Input/input.h"
#include "Graphic/graphic.h"

class GameContext {
 public:
  GameContext() = default;

  InputSystem* GetInput() const {
    return input_;
  }
  void SetInputSystem(InputSystem* input) {
    input_ = input;
  }

  Graphic* GetGraphic() const {
    return graphic_;
  }
  void SetGraphic(Graphic* graphic) {
    graphic_ = graphic;
  }

  EventSystem* GetEventSystem() const {
    return event_system_;
  }
  void SetEventSystem(EventSystem* event_system) {
    event_system_ = event_system;
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

 private:
  InputSystem* input_ = nullptr;
  Graphic* graphic_ = nullptr;
  EventSystem* event_system_ = nullptr;
  AssetManager* asset_manager_ = nullptr;
  DebugDrawer* debug_drawer_ = nullptr;
};
