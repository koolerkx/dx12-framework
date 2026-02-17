#pragma once
#include "Component/behavior_component.h"
#include "Framework/Math/Math.h"

using Math::Vector2;
using Math::Vector3;

class InputSystem;
class TransformComponent;

class FreeCameraController : public BehaviorComponent<FreeCameraController> {
 public:
  struct Props {
    float movement_speed = 10.0f;
    float rotation_speed = 2.0f;
    float smoothness = 10.0f;
  };

  using BehaviorComponent::BehaviorComponent;

  FreeCameraController(GameObject* owner, const Props& props) : BehaviorComponent(owner) {
    SetMovementSpeed(props.movement_speed);
    SetRotationSpeed(props.rotation_speed);
    SetSmoothness(props.smoothness);
  }

  void OnUpdate(float dt) override;
  void OnDebugUpdate(float dt) override;

  void SetMovementSpeed(float speed) {
    movement_speed_ = speed;
  }
  void SetRotationSpeed(float speed) {
    rotation_speed_ = speed;
  }
  void SetSmoothness(float value) {
    smoothness_ = value;
  }
  void SetBounds(const Math::AABB& bounds) {
    bounds_ = bounds;
    has_bounds_ = true;
  }

  float GetMovementSpeed() const {
    return movement_speed_;
  }
  float GetRotationSpeed() const {
    return rotation_speed_;
  }

 private:
  struct InputState {
    Vector2 move_input = {0.0f, 0.0f};
    float vertical_input = 0.0f;
    Vector2 look_input = {0.0f, 0.0f};
  };

  InputState GatherInput(InputSystem* input) const;
  void ApplyMovement(const InputState& input, float dt);
  void ApplyRotation(const InputState& input, float dt);
  void SyncEulerFromTransform(TransformComponent* transform);

 private:
  float movement_speed_ = 10.0f;
  float rotation_speed_ = 2.0f;
  float smoothness_ = 10.0f;

  Vector3 current_euler_ = Vector3::Zero;
  Vector2 velocity_ = Vector2::Zero;
  float vertical_velocity_ = 0.0f;

  Math::AABB bounds_ = {};
  bool has_bounds_ = false;
};
