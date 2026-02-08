#pragma once

#include "Framework/Event/event.hpp"
#include "scene_id.h"

class IScene;

struct SceneChangedEvent : Event<SceneChangedEvent> {
  static constexpr std::string_view EventName = "scene.changed";
  IScene* new_scene = nullptr;
  SceneId scene_id{};
};
