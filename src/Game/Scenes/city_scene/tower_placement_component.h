#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "Component/behavior_component.h"
#include "Framework/Event/event_scope.hpp"
#include "Framework/Math/Math.h"

struct ModelData;
class CameraComponent;
class InputSystem;
class GameObject;
class InstancedModelRenderer;
class NavGrid;

enum class PlacementState : uint8_t { Inactive, Hovering, Selected };

class TowerPlacementComponent : public BehaviorComponent<TowerPlacementComponent> {
 public:
  struct Props {
    NavGrid* nav = nullptr;
  };

  using BehaviorComponent::BehaviorComponent;
  TowerPlacementComponent(GameObject* owner, const Props& props)
      : BehaviorComponent(owner), nav_(props.nav) {}

  void OnStart() override;
  void OnUpdate(float dt) override;

  void Activate(std::function<void()> on_finished);
  void Deactivate();
  bool IsActive() const;

 private:
  void TransitionTo(PlacementState new_state);
  void UpdateHovering();
  void UpdateSelected();
  void UpdateTowerHoverRadar();

  Math::Vector2 SnapToGrid(const Math::Vector2& world_xz) const;
  void CreatePreview(const std::shared_ptr<ModelData>& model);
  void DestroyPreview();
  void UpdatePreviewPosition();
  void PlaceTower();

  Math::AABB ComputePreviewBounds() const;
  Math::AABB ComputeTowerBounds() const;
  bool IsPlacementBlocked() const;
  bool IsOverlappingEnemySpawn() const;
  void UpdateOverlapHighlights();
  void ClearHighlights();
  Math::AABB HideOverlappedInstances();

  struct HighlightedInstance {
    InstancedModelRenderer* renderer;
    std::string instance_id;

    bool operator<(const HighlightedInstance& other) const {
      if (renderer != other.renderer) return renderer < other.renderer;
      return instance_id < other.instance_id;
    }
  };

  std::set<HighlightedInstance> highlighted_instances_;

  PlacementState state_ = PlacementState::Inactive;
  std::function<void()> on_finished_;
  EventScope event_scope_;

  NavGrid* nav_ = nullptr;
  CameraComponent* camera_ = nullptr;
  InputSystem* input_ = nullptr;

  std::shared_ptr<ModelData> selection_a_model_;
  std::shared_ptr<ModelData> selection_b_model_;
  std::shared_ptr<ModelData> tower_model_;

  struct PlacedTower {
    Math::Vector2 grid_xz;
    float range;
    GameObject* game_object = nullptr;
  };

  GameObject* preview_go_ = nullptr;
  GameObject* radar_go_ = nullptr;
  GameObject* hover_radar_go_ = nullptr;
  GameObject* hovered_tower_go_ = nullptr;
  Math::Vector2 hovered_tower_xz_ = {};
  std::vector<PlacedTower> placed_towers_;
  Math::Vector2 snapped_xz_ = {};
  float pulse_time_ = 0.0f;
  int tower_count_ = 0;
};
