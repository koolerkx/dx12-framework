#pragma once
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>

#include "scene.h"
#include "scene_key.h"

class AssetManager;
class GameContext;
class IRenderService;

class SceneManager {
 public:
  template <typename T>
  void Register() {
    constexpr auto key = SceneKeyTrait<T>::KEY;
    factories_[key] = []() { return std::make_unique<T>(); };
    SceneKeyTable::Instance().Register(key, SceneKeyTrait<T>::NAME);
  }

  void RequestLoad(SceneKey key);
  void ProcessPending(AssetManager& asset_manager, GameContext* context, IRenderService* render_service);

  IScene* GetCurrentScene() const {
    return current_scene_.get();
  }

  void Shutdown(IRenderService* render_service);

 private:
  std::unordered_map<SceneKey, std::function<std::unique_ptr<IScene>()>> factories_;
  std::unique_ptr<IScene> current_scene_;
  std::optional<SceneKey> pending_scene_;
};
