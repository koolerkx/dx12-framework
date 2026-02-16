#include "player_control_component.h"

#include "Framework/Input/input.h"
#include "Scenes/city_scene/tower_placement_component.h"
#include "game_context.h"
#include "game_object.h"

void PlayerControlComponent::OnInit() {
  input_ = GetContext()->GetInput();
  GetOwner()->AddComponent<TowerPlacementComponent>(TowerPlacementComponent::Props{.nav = nav_});
}

void PlayerControlComponent::OnUpdate(float /*dt*/) {
  if (mode_ == PlayerMode::Normal && input_->GetKeyDown(Keyboard::KeyCode::F)) {
    EnterPlacingMode();
  }
}

void PlayerControlComponent::EnterPlacingMode() {
  auto* placement = GetOwner()->GetComponent<TowerPlacementComponent>();
  if (!placement) return;
  mode_ = PlayerMode::PlacingTower;
  placement->Activate([this]() { ReturnToNormal(); });
}

void PlayerControlComponent::ReturnToNormal() {
  mode_ = PlayerMode::Normal;
}
