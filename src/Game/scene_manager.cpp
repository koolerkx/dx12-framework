#include "scene_manager.h"

#include "Framework/Event/event_bus.hpp"
#include "Framework/Render/render_service.h"
#include "game_context.h"
#include "scene_events.h"

void SceneManager::RequestLoad(SceneKey key) {
  pending_scene_ = key;
}

void SceneManager::ProcessPending(AssetManager& asset_manager, GameContext* context, IRenderService* render_service) {
  if (!pending_scene_.has_value()) return;

  SceneKey key = pending_scene_.value();
  pending_scene_.reset();

  auto it = factories_.find(key);
  if (it == factories_.end()) return;

  auto new_scene = it->second();

  if (current_scene_) {
    if (render_service) render_service->WaitForGpuIdle();
    current_scene_->Exit();
    current_scene_.reset();
  }

  new_scene->SetContext(context);
  new_scene->ApplyDefaults(context->GetSceneDefaults());
  new_scene->Enter(asset_manager);
  current_scene_ = std::move(new_scene);

  if (auto bus = context->GetEventBus()) {
    bus->Emit(SceneChangedEvent{.new_scene = current_scene_.get(), .scene_key = key});
  }
}

void SceneManager::Shutdown(IRenderService* render_service) {
  if (current_scene_) {
    if (render_service) render_service->WaitForGpuIdle();
    current_scene_->Exit();
    current_scene_.reset();
  }
}
