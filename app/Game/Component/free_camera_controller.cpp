#include "free_camera_controller.h"

#include <algorithm>

#include "Component/transform_component.h"
#include "Framework/Input/input.h"
#include "Framework/Input/keyboard.h"
#include "game_context.h"
#include "game_object.h"

using namespace DirectX;

void FreeCameraController::OnUpdate(float dt) {
  GameContext* context = GetContext();
  if (!context) return;

  InputSystem* input = context->GetInput();
  if (!input) return;

  InputState input_state = GatherInput(input);
  ApplyMovement(input_state, dt);
  ApplyRotation(input_state, dt);
}

FreeCameraController::InputState FreeCameraController::GatherInput(InputSystem* input) const {
  InputState state;

  float horizontal = 0.0f;
  float forward = 0.0f;

  if (input->GetKey(Keyboard::KeyCode::W)) forward += 1.0f;
  if (input->GetKey(Keyboard::KeyCode::S)) forward -= 1.0f;
  if (input->GetKey(Keyboard::KeyCode::D)) horizontal += 1.0f;
  if (input->GetKey(Keyboard::KeyCode::A)) horizontal -= 1.0f;

  state.move_input = {horizontal, forward};

  if (input->GetKey(Keyboard::KeyCode::E)) state.vertical_input += 1.0f;
  if (input->GetKey(Keyboard::KeyCode::Q)) state.vertical_input -= 1.0f;

  float pitch = 0.0f;
  float yaw = 0.0f;

  if (input->GetKey(Keyboard::KeyCode::Up)) pitch -= 1.0f;
  if (input->GetKey(Keyboard::KeyCode::Down)) pitch += 1.0f;
  if (input->GetKey(Keyboard::KeyCode::Left)) yaw -= 1.0f;
  if (input->GetKey(Keyboard::KeyCode::Right)) yaw += 1.0f;

  state.look_input = {pitch, yaw};

  return state;
}

void FreeCameraController::ApplyMovement(const InputState& input, float dt) {
  auto* transform = owner_->GetComponent<TransformComponent>();
  if (!transform) return;

  XMFLOAT3 position = transform->GetPosition();
  XMFLOAT3 forward = transform->GetForward();
  XMFLOAT3 right = transform->GetRight();

  XMVECTOR pos = XMLoadFloat3(&position);
  XMVECTOR fwd = XMLoadFloat3(&forward);
  XMVECTOR rgt = XMLoadFloat3(&right);

  XMVECTOR forward_xz = XMVector3Normalize(fwd * XMVectorSet(1.0f, 0.0f, 1.0f, 0.0f));
  XMVECTOR right_xz = XMVector3Normalize(rgt * XMVectorSet(1.0f, 0.0f, 1.0f, 0.0f));

  float target_horizontal = input.move_input.x;
  float target_forward = input.move_input.y;
  float target_vertical = input.vertical_input;

  float lerp_factor = (std::min)(1.0f, smoothness_ * dt);
  velocity_.x += (target_horizontal - velocity_.x) * lerp_factor;
  velocity_.y += (target_forward - velocity_.y) * lerp_factor;
  vertical_velocity_ += (target_vertical - vertical_velocity_) * lerp_factor;

  XMVECTOR movement = XMVectorZero();
  movement += forward_xz * velocity_.y;
  movement += right_xz * velocity_.x;
  movement += XMVectorSet(0.0f, vertical_velocity_, 0.0f, 0.0f);

  pos += movement * movement_speed_ * dt;

  XMFLOAT3 new_position;
  XMStoreFloat3(&new_position, pos);
  transform->SetPosition(new_position);
}

void FreeCameraController::ApplyRotation(const InputState& input, float dt) {
  auto* transform = owner_->GetComponent<TransformComponent>();
  if (!transform) return;

  current_euler_.x += input.look_input.x * rotation_speed_ * dt;
  current_euler_.y += input.look_input.y * rotation_speed_ * dt;

  constexpr float max_pitch = XM_PIDIV2 - 0.01f;
  current_euler_.x = std::clamp(current_euler_.x, -max_pitch, max_pitch);

  transform->SetRotationEuler(current_euler_.x, current_euler_.y, 0.0f);
}
