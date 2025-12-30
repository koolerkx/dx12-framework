#include "character_mover_component.h"

#include <cmath>

#include "game_object.h"
#include "Component/transform_component.h"

void CharacterMover::OnUpdate(float dt) {
  time_ += dt;

  float x = 500.0f + std::cos(time_) * 75.0f;
  float y = 500.0f + std::sin(time_) * 75.0f;

  auto* transform = GetOwner()->GetComponent<TransformComponent>();
  if (transform) {
    transform->SetPosition({x, y, 0.0f});
  }
}
