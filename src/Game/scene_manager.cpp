#include "scene_manager.h"

#include "Framework/Event/event_bus.hpp"
#include "Graphic/graphic.h"
#include "game_context.h"
#include "scene_events.h"

void SceneManager::RequestLoad(SceneId id) {
  pending_scene_ = id;
}

void SceneManager::ProcessPending(AssetManager& asset_manager, GameContext* context, Graphic* graphic) {
  if (!pending_scene_.has_value()) return;

  SceneId id = pending_scene_.value();
  pending_scene_.reset();

  auto it = factories_.find(id);
  if (it == factories_.end()) return;

  auto new_scene = it->second();

  if (current_scene_) {
    if (graphic) graphic->WaitForGpuIdle();
    current_scene_->Exit();
    current_scene_.reset();
  }

  new_scene->SetContext(context);
  new_scene->ApplyDefaults(context->GetSceneDefaults());
  new_scene->Enter(asset_manager);
  current_scene_ = std::move(new_scene);

  if (auto bus = context->GetEventBus()) {
    bus->Emit(SceneChangedEvent{.new_scene = current_scene_.get(), .scene_id = id});
  }
}

void SceneManager::Shutdown(Graphic* graphic) {
  if (current_scene_) {
    if (graphic) graphic->WaitForGpuIdle();
    current_scene_->Exit();
    current_scene_.reset();
  }
}
