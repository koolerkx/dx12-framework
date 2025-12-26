#pragma once

#include "Graphic/graphic.h"
#include "scene.h"

class Game {
 public:
  Game(Graphic& graphic);
  ~Game() = default;

  void OnUpdate([[maybe_unused]] float dt);
  void OnFixedUpdate([[maybe_unused]] float dt);

 private:
  Graphic& graphic_;
  Scene scene_;
};
