#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <set>
#include <string>

#include "Component/behavior_component.h"
#include "Framework/Math/Math.h"

struct ModelData;
class CameraComponent;
class InputSystem;
class GameObject;
class InstancedModelRenderer;

enum class PlacementState : uint8_t { Inactive, Hovering, Selected };

class TowerPlacementComponent : public BehaviorComponent<TowerPlacementComponent> {
 public:
  using BehaviorComponent::BehaviorComponent;

  void OnStart() override;
  void OnUpdate(float dt) override;

  void Activate(std::function<void()> on_finished);
  void Deactivate();
  bool IsActive() const;

 private:
  void TransitionTo(PlacementState new_state);
  void UpdateHovering();
  void UpdateSelected();

  Math::Vector2 SnapToGrid(const Math::Vector2& world_xz) const;
  void CreatePreview(const std::shared_ptr<ModelData>& model);
  void DestroyPreview();
  void UpdatePreviewPosition();
  void PlaceTower();

  Math::AABB ComputePreviewBounds() const;
  void UpdateOverlapHighlights();
  void ClearHighlights();

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

  CameraComponent* camera_ = nullptr;
  InputSystem* input_ = nullptr;
  float screen_width_ = 0.0f;
  float screen_height_ = 0.0f;

  std::shared_ptr<ModelData> selection_a_model_;
  std::shared_ptr<ModelData> selection_b_model_;

  GameObject* preview_go_ = nullptr;
  Math::Vector2 snapped_xz_ = {};
  float pulse_time_ = 0.0f;
};
