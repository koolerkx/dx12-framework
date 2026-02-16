#include "tower_placement_component.h"

#include "Asset/asset_manager.h"
#include "Component/Renderer/mesh_renderer.h"
#include "Component/camera_component.h"
#include "Component/model_component.h"
#include "Component/transform_component.h"
#include "Framework/Core/color.h"
#include "Framework/Input/input.h"
#include "Map/ground_ray_caster.h"
#include "SceneSetting/active_camera_setting.h"
#include "game_context.h"
#include "game_object.h"
#include "scene.h"

void TowerPlacementComponent::OnStart() {
  auto* context = GetContext();
  input_ = context->GetInput();
  screen_width_ = static_cast<float>(context->GetGraphic()->GetFrameBufferWidth());
  screen_height_ = static_cast<float>(context->GetGraphic()->GetFrameBufferHeight());

  auto& assets = context->GetAssetManager();
  constexpr float FBX_UNIT_SCALE = 0.01f;
  selection_a_model_ = assets.LoadModel("Content/models/tower/selection-a.fbx", FBX_UNIT_SCALE);
  selection_b_model_ = assets.LoadModel("Content/models/tower/selection-b.fbx", FBX_UNIT_SCALE);
}

void TowerPlacementComponent::OnUpdate(float dt) {
  if (state_ == PlacementState::Inactive) return;

  if (input_->GetKeyDown(Keyboard::KeyCode::Escape)) {
    Deactivate();
    return;
  }

  pulse_time_ += dt;

  switch (state_) {
    case PlacementState::Hovering:
      UpdateHovering();
      break;
    case PlacementState::Selected:
      UpdateSelected();
      break;
    default:
      break;
  }
}

void TowerPlacementComponent::Activate(std::function<void()> on_finished) {
  on_finished_ = std::move(on_finished);
  camera_ = GetOwner()->GetScene()->GetCameraSetting().GetActive();
  pulse_time_ = 0.0f;
  TransitionTo(PlacementState::Hovering);
  CreatePreview(selection_a_model_);
}

void TowerPlacementComponent::Deactivate() {
  DestroyPreview();
  TransitionTo(PlacementState::Inactive);
  if (on_finished_) {
    auto callback = std::move(on_finished_);
    on_finished_ = nullptr;
    callback();
  }
}

bool TowerPlacementComponent::IsActive() const {
  return state_ != PlacementState::Inactive;
}

void TowerPlacementComponent::TransitionTo(PlacementState new_state) {
  state_ = new_state;
}

void TowerPlacementComponent::UpdateHovering() {
  auto [mx, my] = input_->GetMousePosition();
  if (!camera_) return;

  auto hit = GroundRayCaster::ScreenToGroundXZ(mx, my, screen_width_, screen_height_, camera_->GetCameraData());
  if (hit) {
    snapped_xz_ = SnapToGrid(*hit);
    UpdatePreviewPosition();
  }

  if (input_->GetMouseButtonDown(Mouse::Button::Right)) {
    Deactivate();
    return;
  }

  if (input_->GetMouseButtonDown(Mouse::Button::Left)) {
    DestroyPreview();
    CreatePreview(selection_b_model_);
    UpdatePreviewPosition();
    TransitionTo(PlacementState::Selected);
  }
}

void TowerPlacementComponent::UpdateSelected() {
  if (input_->GetMouseButtonDown(Mouse::Button::Right)) {
    DestroyPreview();
    CreatePreview(selection_a_model_);
    UpdatePreviewPosition();
    TransitionTo(PlacementState::Hovering);
    return;
  }

  if (input_->GetKeyDown(Keyboard::KeyCode::Space)) {
    PlaceTower();
    Deactivate();
  }
}

Math::Vector2 TowerPlacementComponent::SnapToGrid(const Math::Vector2& world_xz) const {
  constexpr float GRID_OFFSET_X = 0.5f;
  constexpr float GRID_OFFSET_Z = 0.25f;
  float snapped_x = std::floor(world_xz.x - GRID_OFFSET_X + 0.5f) + GRID_OFFSET_X;
  float snapped_z = std::floor(world_xz.y - GRID_OFFSET_Z + 0.5f) + GRID_OFFSET_Z;
  return Math::Vector2(snapped_x, snapped_z);
}

void TowerPlacementComponent::CreatePreview(const std::shared_ptr<ModelData>& model) {
  if (!model) return;
  auto* scene = GetOwner()->GetScene();
  preview_go_ = scene->CreateGameObject("TowerPreview");
  preview_go_->SetTransient(true);
  preview_go_->AddComponent<ModelComponent>(ModelComponent::Props{.model = model});
}

void TowerPlacementComponent::DestroyPreview() {
  if (preview_go_) {
    preview_go_->Destroy();
    preview_go_ = nullptr;
  }
}

void TowerPlacementComponent::UpdatePreviewPosition() {
  if (!preview_go_) return;
  auto* transform = preview_go_->GetTransform();
  transform->SetPosition({snapped_xz_.x, 0.0f, snapped_xz_.y});

  if (state_ == PlacementState::Hovering) {
    constexpr float SCALE_MIN = 0.75f;
    constexpr float SCALE_MAX = 0.9f;
    constexpr float PULSE_SPEED = 3.0f;
    float t = (Math::Sin(pulse_time_ * PULSE_SPEED) + 1.0f) * 0.5f;
    float scale_xz = Math::Lerp(SCALE_MIN, SCALE_MAX, t);
    transform->SetScale({scale_xz, 1.0f, scale_xz});
  }
}

void TowerPlacementComponent::PlaceTower() {
  constexpr float TOWER_HALF_HEIGHT = 0.5f;
  auto* scene = GetOwner()->GetScene();
  auto* tower =
    scene->CreateGameObject("Tower", {.position = {snapped_xz_.x, TOWER_HALF_HEIGHT, snapped_xz_.y}, .scale = {0.75f, 0.75f, 0.75f}});
  tower->AddComponent<MeshRenderer>(MeshRenderer::Props{
    .mesh_type = DefaultMesh::Cube,
    .color = colors::Green,
  });
}
