#pragma once
#include <DirectXMath.h>

#include "Component/component.h"

class CharacterMover : public Component<CharacterMover> {
 public:
  using Component::Component;

  void OnUpdate(float dt) override;  // Game logic here!

 private:
  float time_ = 0.0f;
};
