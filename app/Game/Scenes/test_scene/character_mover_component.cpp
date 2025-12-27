#include "character_mover_component.h"

#include <cmath>

#include "game_object.h"
#include "Component/transform_component.h"

void CharacterMover::OnUpdate(float dt) {
  time_ += dt;

  float x = 400.0f + std::cos(time_) * 100.0f;
  float y = 300.0f + std::sin(time_) * 100.0f;

  auto* transform = GetOwner()->GetComponent<TransformComponent>();
  if (transform) {
    transform->SetPosition({x, y, 0.0f});
  }
}
