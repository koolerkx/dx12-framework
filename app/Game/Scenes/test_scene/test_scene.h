#pragma once
#include <memory>

#include "scene.h"

struct Texture;

class TestScene : public IScene {
 public:
  void OnEnter(Graphic& graphic) override;
  void OnExit() override;

 private:
  std::shared_ptr<Texture> texture_background_;
  std::shared_ptr<Texture> texture_character_;
  GameObject* character_object_ = nullptr;
};
