#include "tower_placement_component.h"

#include "Asset/asset_manager.h"
#include "Component/Collider/box_collider_component.h"
#include "Component/Renderer/instanced_model_renderer.h"
#include "Component/Renderer/mesh_renderer.h"
#include "Component/camera_component.h"
#include "Component/model_component.h"
#include "Component/transform_component.h"
#include "Framework/Core/color.h"
#include "Framework/Input/input.h"
#include "Map/ground_ray_caster.h"
#include "Map/nav_grid.h"
#include "Map/nav_grid_events.h"
#include "Scenes/city_scene/city_scene_config.h"
#include "SceneSetting/active_camera_setting.h"
#include "Scenes/city_scene/tower_component.h"
#include "game_context.h"
#include "game_object.h"
#include "scene.h"

namespace {

struct TowerPlacementConfig {
  float grid_snap_offset_x = 0.5f;
  float grid_snap_offset_z = 0.25f;
  float pulse_scale_min = 0.75f;
  float pulse_scale_max = 0.9f;
  float pulse_speed = 3.0f;
  float tower_half_extent = 0.375f;
  float tower_half_height = 0.5f;
  float tower_scale = 0.75f;
};

constexpr TowerPlacementConfig TOWER_CFG;

}  // namespace

void TowerPlacementComponent::OnStart() {
  auto* context = GetContext();
  input_ = context->GetInput();

  namespace cfg = CitySceneConfig;
  auto& assets = context->GetAssetManager();
  selection_a_model_ = assets.LoadModel(cfg::PATHS.tower_selection_a, cfg::FBX_UNIT_SCALE);
  selection_b_model_ = assets.LoadModel(cfg::PATHS.tower_selection_b, cfg::FBX_UNIT_SCALE);
  tower_model_ = assets.LoadModel(cfg::PATHS.tower_model, cfg::FBX_UNIT_SCALE);
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
  ClearHighlights();
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

  auto* gfx = GetContext()->GetGraphic();
  float screen_w = static_cast<float>(gfx->GetFrameBufferWidth());
  float screen_h = static_cast<float>(gfx->GetFrameBufferHeight());
  auto hit = GroundRayCaster::ScreenToGroundXZ(mx, my, screen_w, screen_h, camera_->GetCameraData());
  if (hit) {
    snapped_xz_ = SnapToGrid(*hit);
    UpdatePreviewPosition();
    UpdateOverlapHighlights();
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
  float snapped_x = std::floor(world_xz.x - TOWER_CFG.grid_snap_offset_x + 0.5f) + TOWER_CFG.grid_snap_offset_x;
  float snapped_z = std::floor(world_xz.y - TOWER_CFG.grid_snap_offset_z + 0.5f) + TOWER_CFG.grid_snap_offset_z;
  return Math::Vector2(snapped_x, snapped_z);
}

void TowerPlacementComponent::CreatePreview(const std::shared_ptr<ModelData>& model) {
  if (!model) return;
  auto* scene = GetOwner()->GetScene();
  preview_go_ = scene->CreateGameObject("TowerPreview_" + std::to_string(tower_count_));
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
    float t = (Math::Sin(pulse_time_ * TOWER_CFG.pulse_speed) + 1.0f) * 0.5f;
    float scale_xz = Math::Lerp(TOWER_CFG.pulse_scale_min, TOWER_CFG.pulse_scale_max, t);
    transform->SetScale({scale_xz, 1.0f, scale_xz});
  }
}

void TowerPlacementComponent::PlaceTower() {
  Math::AABB removed_area = HideOverlappedInstances();

  auto* scene = GetOwner()->GetScene();
  auto* tower =
    scene->CreateGameObject("Tower_" + std::to_string(tower_count_++), {.position = {snapped_xz_.x, 0.0f, snapped_xz_.y}});
  tower->AddComponent<ModelComponent>(ModelComponent::Props{.model = tower_model_});
  tower->AddComponent<TowerComponent>(TowerComponent::Props{});

  if (nav_) {
    float he = TOWER_CFG.tower_half_extent;
    Math::AABB tower_bounds = {
      {snapped_xz_.x - he, 0.0f, snapped_xz_.y - he},
      {snapped_xz_.x + he, 1.0f, snapped_xz_.y + he},
    };
    nav_->BlockArea(tower_bounds);

    Math::AABB affected_area = tower_bounds;
    if (removed_area.min.x <= removed_area.max.x) {
      affected_area.Encapsulate(removed_area);
    }
    GetContext()->GetEventBus()->Emit(NavGridChangedEvent{.affected_area = affected_area});
  }
}

Math::AABB TowerPlacementComponent::ComputePreviewBounds() const {
  if (!selection_a_model_) return {};
  Math::Vector3 offset(snapped_xz_.x, 0.0f, snapped_xz_.y);
  return {selection_a_model_->bounds.min + offset, selection_a_model_->bounds.max + offset};
}

static std::string ExtractRendererGoName(const std::string& instance_id) {
  auto last_underscore = instance_id.rfind('_');
  if (last_underscore == std::string::npos) return instance_id + "_instances";
  return instance_id.substr(0, last_underscore) + "_instances";
}

void TowerPlacementComponent::UpdateOverlapHighlights() {
  auto* scene = GetOwner()->GetScene();
  auto* object_go = scene->FindGameObject("object");
  if (!object_go) return;

  Math::AABB preview_bounds = ComputePreviewBounds();
  std::set<HighlightedInstance> new_highlights;

  for (auto* child : object_go->GetChildren()) {
    auto* collider = child->GetComponent<BoxColliderComponent>();
    if (!collider) continue;

    if (!preview_bounds.Intersects(collider->GetWorldBounds())) continue;

    const auto& collider_name = child->GetName();
    constexpr std::string_view COLLIDER_SUFFIX = "_collider";
    if (collider_name.size() <= COLLIDER_SUFFIX.size()) continue;

    std::string instance_id = collider_name.substr(0, collider_name.size() - COLLIDER_SUFFIX.size());
    std::string renderer_go_name = ExtractRendererGoName(instance_id);

    auto* renderer_go = object_go->FindChild(renderer_go_name);
    if (!renderer_go) continue;

    auto* renderer = renderer_go->GetComponent<InstancedModelRenderer>();
    if (!renderer) continue;

    new_highlights.insert({renderer, instance_id});
  }

  static const Math::Vector4 RED_OVERLAY = {1.0f, 0.0f, 0.0f, 0.5f};
  static const Math::Vector4 NO_OVERLAY = {0.0f, 0.0f, 0.0f, 0.0f};

  for (const auto& prev : highlighted_instances_) {
    if (new_highlights.find(prev) == new_highlights.end()) {
      prev.renderer->UpdateById(prev.instance_id, [](const InstanceProps& p) {
        InstanceProps updated = p;
        updated.overlay_color = NO_OVERLAY;
        return updated;
      });
    }
  }

  for (const auto& cur : new_highlights) {
    if (highlighted_instances_.find(cur) == highlighted_instances_.end()) {
      cur.renderer->UpdateById(cur.instance_id, [](const InstanceProps& p) {
        InstanceProps updated = p;
        updated.overlay_color = RED_OVERLAY;
        return updated;
      });
    }
  }

  highlighted_instances_ = std::move(new_highlights);
}

Math::AABB TowerPlacementComponent::HideOverlappedInstances() {
  auto* scene = GetOwner()->GetScene();
  auto* object_go = scene->FindGameObject("object");

  Math::AABB merged_area = {{1e9f, 1e9f, 1e9f}, {-1e9f, -1e9f, -1e9f}};

  for (const auto& entry : highlighted_instances_) {
    entry.renderer->UpdateById(entry.instance_id, [](const InstanceProps& p) {
      InstanceProps updated = p;
      updated.visible = false;
      updated.overlay_color = {0.0f, 0.0f, 0.0f, 0.0f};
      return updated;
    });

    if (object_go) {
      std::string collider_name = entry.instance_id + "_collider";
      auto* collider_go = object_go->FindChild(collider_name);
      if (collider_go) {
        if (nav_) {
          auto* collider = collider_go->GetComponent<BoxColliderComponent>();
          if (collider) {
            Math::AABB bounds = collider->GetWorldBounds();
            nav_->UnblockArea(bounds);
            merged_area.Encapsulate(bounds);
          }
        }
        collider_go->Destroy();
      }
    }
  }
  highlighted_instances_.clear();
  return merged_area;
}

void TowerPlacementComponent::ClearHighlights() {
  static const Math::Vector4 NO_OVERLAY = {0.0f, 0.0f, 0.0f, 0.0f};
  for (const auto& entry : highlighted_instances_) {
    entry.renderer->UpdateById(entry.instance_id, [](const InstanceProps& p) {
      InstanceProps updated = p;
      updated.overlay_color = NO_OVERLAY;
      return updated;
    });
  }
  highlighted_instances_.clear();
}
