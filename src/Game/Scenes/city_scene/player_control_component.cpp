#include "player_control_component.h"

#include <limits>

#include "Component/Collider/sphere_collider_component.h"
#include "Component/camera_component.h"
#include "Framework/Input/input.h"
#include "Framework/Math/Math.h"
#include "Map/ground_ray_caster.h"
#include "SceneSetting/active_camera_setting.h"
#include "Scenes/city_scene/city_scene_events.h"
#include "Scenes/city_scene/enemy_component.h"
#include "Scenes/city_scene/hud_manager_component.h"
#include "Scenes/city_scene/pulse_path_component.h"
#include "Scenes/city_scene/tower_placement_component.h"
#include "game_context.h"
#include "game_object.h"
#include "scene.h"

void PlayerControlComponent::OnInit() {
  input_ = GetContext()->GetInput();
  GetOwner()->AddComponent<TowerPlacementComponent>(TowerPlacementComponent::Props{.nav = nav_});

  auto* pulse_go = GetOwner()->GetScene()->CreateGameObject("PulsePath");
  pulse_go->SetParent(GetOwner());
  pulse_path_ = pulse_go->AddComponent<PulsePathComponent>(PulsePathComponent::Props{});

  event_scope_.Subscribe<ToggleTowerPlacementEvent>(
    *GetContext()->GetEventBus(), [this](const ToggleTowerPlacementEvent&) {
      if (mode_ == PlayerMode::Normal) {
        ClearEnemyHover();
        EnterPlacingMode();
      } else if (mode_ == PlayerMode::PlacingTower) {
        auto* placement = GetOwner()->GetComponent<TowerPlacementComponent>();
        if (placement && placement->IsActive()) {
          placement->Deactivate();
        }
      }
    });
}

void PlayerControlComponent::OnUpdate(float /*dt*/) {
  if (mode_ == PlayerMode::Normal) {
    UpdateEnemyHover();
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

void PlayerControlComponent::UpdateEnemyHover() {
  if (hovered_enemy_ && hovered_enemy_->IsPendingDestroy()) {
    pulse_path_->Hide();
    hovered_enemy_ = nullptr;
  }

  auto* camera = GetOwner()->GetScene()->GetCameraSetting().GetActive();
  auto* enemy_manager = GetOwner()->GetScene()->FindGameObject("EnemyManager");
  if (!enemy_manager || !camera) return;

  float screen_w = static_cast<float>(GetContext()->GetGraphic()->GetFrameBufferWidth());
  float screen_h = static_cast<float>(GetContext()->GetGraphic()->GetFrameBufferHeight());

  auto [mx, my] = input_->GetMousePosition();
  Math::Ray ray = GroundRayCaster::ScreenToWorldRay(mx, my, screen_w, screen_h, camera->GetCameraData());

  GameObject* nearest = nullptr;
  float nearest_dist = (std::numeric_limits<float>::max)();

  for (auto* child : enemy_manager->GetChildren()) {
    if (child->IsPendingDestroy()) continue;
    if (!child->GetComponent<EnemyComponent>()) continue;

    auto* collider = child->GetComponent<SphereColliderComponent>();
    if (!collider) continue;

    float dist = 0.0f;
    if (Math::Intersects(ray, collider->GetWorldSphere(), dist) && dist < nearest_dist) {
      nearest_dist = dist;
      nearest = child;
    }
  }

  if (nearest == hovered_enemy_) return;

  ClearEnemyHover();

  if (nearest) {
    hovered_enemy_ = nearest;
    pulse_path_->Show(nearest, pulse_params_);
  }
}

void PlayerControlComponent::ClearEnemyHover() {
  if (hovered_enemy_) {
    pulse_path_->Hide();
    hovered_enemy_ = nullptr;
  }
}
