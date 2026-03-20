#pragma once
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>

#include "scene.h"
#include "scene_id.h"

class AssetManager;
class GameContext;
class IRenderService;

class SceneManager {
 public:
  template <typename T>
  void Register(SceneId id) {
    factories_[id] = []() { return std::make_unique<T>(); };
  }

  void RequestLoad(SceneId id);
  void ProcessPending(AssetManager& asset_manager, GameContext* context, IRenderService* render_service);

  IScene* GetCurrentScene() const {
    return current_scene_.get();
  }

  void Shutdown(IRenderService* render_service);

 private:
  std::unordered_map<SceneId, std::function<std::unique_ptr<IScene>()>> factories_;
  std::unique_ptr<IScene> current_scene_;
  std::optional<SceneId> pending_scene_;
};
