#pragma once
#include "Component/component.h"

class InputSystem;

class FreeCameraController : public Component<FreeCameraController> {
 public:
  using Component::Component;

  void OnUpdate(float dt) override;

  void SetMovementSpeed(float speed) {
    movement_speed_ = speed;
  }
  void SetRotationSpeed(float speed) {
    rotation_speed_ = speed;
  }
  void SetSmoothness(float value) {
    smoothness_ = value;
  }

  float GetMovementSpeed() const {
    return movement_speed_;
  }
  float GetRotationSpeed() const {
    return rotation_speed_;
  }

 private:
  struct InputState {
    DirectX::XMFLOAT2 move_input = {0.0f, 0.0f};
    float vertical_input = 0.0f;
    DirectX::XMFLOAT2 look_input = {0.0f, 0.0f};
  };

  InputState GatherInput(InputSystem* input) const;
  void ApplyMovement(const InputState& input, float dt);
  void ApplyRotation(const InputState& input, float dt);

 private:
  float movement_speed_ = 10.0f;
  float rotation_speed_ = 2.0f;
  float smoothness_ = 10.0f;

  DirectX::XMFLOAT3 current_euler_ = {0.0f, 0.0f, 0.0f};
  DirectX::XMFLOAT2 velocity_ = {0.0f, 0.0f};
  float vertical_velocity_ = 0.0f;
};
