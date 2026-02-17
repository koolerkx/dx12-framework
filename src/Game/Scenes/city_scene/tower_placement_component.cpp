#include "tower_placement_component.h"

#include "Asset/asset_manager.h"
#include "Component/Collider/box_collider_component.h"
#include "Component/Collider/sphere_collider_component.h"
#include "Component/Renderer/instanced_model_renderer.h"
#include "Component/Renderer/mesh_renderer.h"
#include "Component/camera_component.h"
#include "Component/model_component.h"
#include "Component/transform_component.h"
#include "Framework/Core/color.h"
#include "Framework/Input/input.h"
#include "Graphic/Pipeline/pixel_shader_descriptors.h"
#include "Map/ground_ray_caster.h"
#include "Map/nav_grid.h"
#include "Map/nav_grid_events.h"
#include "Component/enemy_spawn_component.h"
#include "Framework/Event/event_bus.hpp"
#include "Scenes/city_scene/city_scene_config.h"
#include "Scenes/city_scene/city_scene_events.h"
#include "Scenes/city_scene/floating_text_effect.h"
#include "Scenes/city_scene/enemy_component.h"
#include "Scenes/city_scene/game_state_manager_component.h"
#include "Scenes/city_scene/hud_manager_component.h"
#include "SceneSetting/active_camera_setting.h"
#include "Scenes/city_scene/tower_component.h"
#include "game_context.h"
#include "game_object.h"
#include "scene.h"

namespace {

constexpr CitySceneConfig::TowerPlacementConfig TOWER_CFG;

struct RadarColor {
  float r, g, b;
};

constexpr RadarColor RADAR_COLOR_PREVIEW = {0.3f, 0.5f, 1.0f};
constexpr RadarColor RADAR_COLOR_PLACED = {0.2f, 1.0f, 0.3f};

GameObject* CreateRadarDisc(IScene* scene, const Math::Vector3& world_pos, float range, RadarColor color) {
  auto* radar = scene->CreateGameObject("RadarDisc");
  radar->SetTransient(true);

  float diameter = range * 2.0f;
  radar->GetTransform()->SetPosition({world_pos.x, TOWER_CFG.radar_y_offset, world_pos.z});
  radar->GetTransform()->SetScale({diameter, 1.0f, diameter});

  auto* renderer = radar->AddComponent<MeshRenderer>(MeshRenderer::Props{
    .mesh_type = DefaultMesh::Plane,
  });

  Graphics::RadarRangeShader::Params params{
    .radar_r = color.r, .radar_g = color.g, .radar_b = color.b,
    .scan_speed = 0.4f,
    .ring_count = 4.0f,
    .opacity = 0.5f,
    .emissive_intensity = 3.0f,
    .ring_width = 2.0f,
  };
  renderer->SetShaderWithParams<Graphics::RadarRangeShader>(params);

  return radar;
}

}  // namespace

void TowerPlacementComponent::OnStart() {
  auto* context = GetContext();
  input_ = context->GetInput();

  namespace cfg = CitySceneConfig;
  auto& assets = context->GetAssetManager();
  selection_a_model_ = assets.LoadModel(cfg::PATHS.tower_selection_a, cfg::FBX_UNIT_SCALE);
  selection_b_model_ = assets.LoadModel(cfg::PATHS.tower_selection_b, cfg::FBX_UNIT_SCALE);
  tower_model_ = assets.LoadModel(cfg::PATHS.tower_model, cfg::FBX_UNIT_SCALE);

  auto& bus = *context->GetEventBus();

  event_scope_.Subscribe<TowerPlacementConfirmedEvent>(bus, [this](const TowerPlacementConfirmedEvent&) {
    if (state_ != PlacementState::Selected) return;
    if (IsPlacementBlocked()) {
      GetContext()->GetEventBus()->Emit(OverlapEnemyEvent{});
      return;
    }
    const CitySceneConfig::GoldConfig gold_cfg;
    int total_cost = gold_cfg.ComputePlacementCost(static_cast<int>(highlighted_instances_.size()));
    auto* player = GetOwner()->GetScene()->FindGameObject("Player");
    auto* gold = player ? player->GetComponent<GameStateManagerComponent>() : nullptr;
    if (gold && gold->TrySpendGold(total_cost)) {
      const CitySceneConfig::FloatingTextConfig txt_cfg;
      CitySceneEffect::SpawnCostText(GetOwner()->GetScene(),
        {snapped_xz_.x, txt_cfg.y_offset, snapped_xz_.y}, total_cost);
      PlaceTower();
      Deactivate();
    }
  });

  event_scope_.Subscribe<TowerPlacementCancelledEvent>(bus, [this](const TowerPlacementCancelledEvent&) {
    if (state_ != PlacementState::Selected) return;
    DestroyPreview();
    CreatePreview(selection_a_model_);
    UpdatePreviewPosition();
    TransitionTo(PlacementState::Hovering);
  });
}

void TowerPlacementComponent::OnUpdate(float dt) {
  UpdateTowerHoverRadar();

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
  GetContext()->GetEventBus()->Emit(TowerPlacementExitedEvent{});
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

  auto* hud_go = GetOwner()->GetScene()->FindGameObject("HUD");
  auto* hud = hud_go ? hud_go->GetComponent<HudManagerComponent>() : nullptr;
  bool over_ui = hud && hud->IsMouseOverUI(mx, my);

  auto* gfx = GetContext()->GetGraphic();
  float screen_w = static_cast<float>(gfx->GetFrameBufferWidth());
  float screen_h = static_cast<float>(gfx->GetFrameBufferHeight());
  auto hit = GroundRayCaster::ScreenToGroundXZ(mx, my, screen_w, screen_h, camera_->GetCameraData());
  if (hit) {
    snapped_xz_ = SnapToGrid(*hit);
    UpdatePreviewPosition();
    UpdateOverlapHighlights();
  }

  if (!over_ui && input_->GetMouseButtonDown(Mouse::Button::Right)) {
    Deactivate();
    return;
  }

  if (!over_ui && input_->GetMouseButtonDown(Mouse::Button::Left)) {
    if (IsOverlappingEnemySpawn()) {
      GetContext()->GetEventBus()->Emit(OverlapEnemySpawnEvent{});
      return;
    }
    DestroyPreview();
    CreatePreview(selection_b_model_);
    UpdatePreviewPosition();
    TransitionTo(PlacementState::Selected);

    const CitySceneConfig::GoldConfig gold_cfg;
    int total_cost = gold_cfg.ComputePlacementCost(static_cast<int>(highlighted_instances_.size()));
    GetContext()->GetEventBus()->Emit(TowerPlacementSelectedEvent{.cost = total_cost});
  }
}

void TowerPlacementComponent::UpdateSelected() {
  auto [mx, my] = input_->GetMousePosition();
  auto* hud_go = GetOwner()->GetScene()->FindGameObject("HUD");
  auto* hud = hud_go ? hud_go->GetComponent<HudManagerComponent>() : nullptr;
  bool over_ui = hud && hud->IsMouseOverUI(mx, my);

  if (!over_ui && input_->GetMouseButtonDown(Mouse::Button::Right)) {
    GetContext()->GetEventBus()->Emit(TowerPlacementCancelledEvent{});
    return;
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

  Math::Vector3 pos = {snapped_xz_.x, 0.0f, snapped_xz_.y};
  radar_go_ = CreateRadarDisc(scene, pos, TowerComponent::Props{}.range, RADAR_COLOR_PREVIEW);
}

void TowerPlacementComponent::DestroyPreview() {
  if (radar_go_) {
    radar_go_->Destroy();
    radar_go_ = nullptr;
  }
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
  } else {
    transform->SetScale({1.0f, 1.0f, 1.0f});
  }

  if (radar_go_) {
    radar_go_->GetTransform()->SetPosition({snapped_xz_.x, TOWER_CFG.radar_y_offset, snapped_xz_.y});
  }
}

void TowerPlacementComponent::PlaceTower() {
  Math::AABB removed_area = HideOverlappedInstances();

  auto* scene = GetOwner()->GetScene();
  float s = TOWER_CFG.tower_scale;
  auto* tower =
    scene->CreateGameObject("Tower_" + std::to_string(tower_count_++), {.position = {snapped_xz_.x, 0.0f, snapped_xz_.y}, .scale = {s, s, s}});
  tower->AddComponent<ModelComponent>(ModelComponent::Props{.model = tower_model_});
  tower->AddComponent<TowerComponent>(TowerComponent::Props{});
  placed_towers_.push_back({snapped_xz_, TowerComponent::Props{}.range, tower});

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

Math::AABB TowerPlacementComponent::ComputeTowerBounds() const {
  float he = TOWER_CFG.tower_half_extent;
  float hh = TOWER_CFG.tower_half_height;
  return {
    {snapped_xz_.x - he, 0.0f, snapped_xz_.y - he},
    {snapped_xz_.x + he, hh * 2.0f, snapped_xz_.y + he},
  };
}

bool TowerPlacementComponent::IsOverlappingEnemySpawn() const {
  auto* scene = GetOwner()->GetScene();
  if (!scene) return false;

  Math::AABB tower_bounds = ComputeTowerBounds();
  for (const auto& go : scene->GetGameObjects()) {
    if (go->GetComponent<EnemySpawnComponent>()) {
      auto spawn_pos = go->GetTransform()->GetWorldPosition();
      constexpr CitySceneConfig::SpawnCubeConfig SPAWN;
      float half = SPAWN.scale * 0.5f;
      Math::AABB spawn_bounds = {
        {spawn_pos.x - half, spawn_pos.y - half, spawn_pos.z - half},
        {spawn_pos.x + half, spawn_pos.y + half, spawn_pos.z + half},
      };
      if (tower_bounds.Intersects(spawn_bounds)) return true;
    }
  }
  return false;
}

bool TowerPlacementComponent::IsPlacementBlocked() const {
  auto* scene = GetOwner()->GetScene();
  if (!scene) return true;

  Math::AABB tower_bounds = ComputeTowerBounds();
  auto* enemy_manager = scene->FindGameObject("EnemyManager");
  if (enemy_manager) {
    for (auto* child : enemy_manager->GetChildren()) {
      if (child->IsPendingDestroy()) continue;
      if (!child->GetComponent<EnemyComponent>()) continue;
      auto* sphere = child->GetComponent<SphereColliderComponent>();
      if (sphere && tower_bounds.Intersects(sphere->GetWorldBounds())) return true;
    }
  }
  return false;
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

void TowerPlacementComponent::UpdateTowerHoverRadar() {
  if (placed_towers_.empty()) return;

  auto* cam = GetOwner()->GetScene()->GetCameraSetting().GetActive();
  if (!cam) return;

  auto [mx, my] = input_->GetMousePosition();

  auto* hud_go = GetOwner()->GetScene()->FindGameObject("HUD");
  auto* hud = hud_go ? hud_go->GetComponent<HudManagerComponent>() : nullptr;
  if (hud && hud->IsMouseOverUI(mx, my)) return;
  auto* gfx = GetContext()->GetGraphic();
  float screen_w = static_cast<float>(gfx->GetFrameBufferWidth());
  float screen_h = static_cast<float>(gfx->GetFrameBufferHeight());
  auto hit = GroundRayCaster::ScreenToGroundXZ(mx, my, screen_w, screen_h, cam->GetCameraData());
  auto unhover_tower = [this]() {
    if (hovered_tower_go_) {
      if (auto* tc = hovered_tower_go_->GetComponent<TowerComponent>()) tc->SetHighlighted(false);
      hovered_tower_go_ = nullptr;
    }
  };

  if (!hit) {
    if (hover_radar_go_) {
      hover_radar_go_->Destroy();
      hover_radar_go_ = nullptr;
    }
    unhover_tower();
    return;
  }

  Math::Vector2 snapped = SnapToGrid(*hit);
  constexpr float EPSILON = 0.01f;

  for (const auto& tower : placed_towers_) {
    float dx = snapped.x - tower.grid_xz.x;
    float dz = snapped.y - tower.grid_xz.y;
    if (dx * dx + dz * dz < EPSILON) {
      if (hover_radar_go_ && hovered_tower_xz_.x == tower.grid_xz.x && hovered_tower_xz_.y == tower.grid_xz.y) {
        return;
      }
      if (hover_radar_go_) {
        hover_radar_go_->Destroy();
      }
      unhover_tower();
      Math::Vector3 pos = {tower.grid_xz.x, 0.0f, tower.grid_xz.y};
      hover_radar_go_ = CreateRadarDisc(GetOwner()->GetScene(), pos, tower.range, RADAR_COLOR_PLACED);
      hovered_tower_xz_ = tower.grid_xz;
      hovered_tower_go_ = tower.game_object;
      if (auto* tc = hovered_tower_go_->GetComponent<TowerComponent>()) tc->SetHighlighted(true);
      return;
    }
  }

  if (hover_radar_go_) {
    hover_radar_go_->Destroy();
    hover_radar_go_ = nullptr;
  }
  unhover_tower();
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
