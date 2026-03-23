#pragma once

#include "Framework/Event/event.hpp"
#include "scene_key.h"

class IScene;

struct SceneChangedEvent : Event<SceneChangedEvent> {
  static constexpr std::string_view EventName = "scene.changed";
  IScene* new_scene = nullptr;
  SceneKey scene_key;
};

struct GameOverEvent : Event<GameOverEvent> {
  static constexpr std::string_view EventName = "game.over";
  int wave = 0;
  int kill_count = 0;
};
