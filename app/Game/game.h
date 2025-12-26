#pragma once

#include <memory>

#include "Graphic/graphic.h"
#include "scene.h"

class Game {
 public:
  Game(Graphic& graphic);
  ~Game();

  void Initialize();
  void Shutdown() {};

  void OnUpdate(float dt);
  void OnFixedUpdate(float dt);
  void OnRender();

 private:
  Graphic& graphic_;

  std::unique_ptr<IScene> current_scene_;

  RenderWorld render_world_;
};
