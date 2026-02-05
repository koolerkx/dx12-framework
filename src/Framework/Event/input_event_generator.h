#pragma once

#include <cmath>

#include "Event/event_bus.hpp"
#include "Input/input.h"
#include "input_events.h"

class InputEventGenerator {
 public:
  InputEventGenerator(InputSystem& input, EventBus& bus) : input_(input), bus_(bus) {
  }

  void Update() {
    GenerateKeyboardEvents();
    GenerateMouseEvents();
    GenerateGamepadEvents();
  }

 private:
  void GenerateKeyboardEvents() {
    auto keys_down = input_.GetKeysDown();
    for (auto key : keys_down) {
      int vk = Keyboard::KeyCodeToVirtualKey(key);
      bus_.Emit(KeyDownEvent{.key = key,
        .virtual_key = vk,
        .is_repeat = false,
        .shift_pressed = input_.GetKey(VK_SHIFT),
        .ctrl_pressed = input_.GetKey(VK_CONTROL),
        .alt_pressed = input_.GetKey(VK_MENU)});
    }

    auto keys_up = input_.GetKeysUp();
    for (auto key : keys_up) {
      int vk = Keyboard::KeyCodeToVirtualKey(key);
      bus_.Emit(KeyUpEvent{.key = key,
        .virtual_key = vk,
        .shift_pressed = input_.GetKey(VK_SHIFT),
        .ctrl_pressed = input_.GetKey(VK_CONTROL),
        .alt_pressed = input_.GetKey(VK_MENU)});
    }
  }

  void GenerateMouseEvents() {
    for (int btn = 0; btn < 5; ++btn) {
      Mouse::Button button = static_cast<Mouse::Button>(btn);

      if (input_.GetMouseButtonDown(button)) {
        auto [x, y] = input_.GetMousePosition();
        bus_.Emit(MouseButtonDownEvent{
          .button = button,
          .x = x,
          .y = y,
          .click_count = 1  // TODO: Implement double-click detection
        });
      }

      if (input_.GetMouseButtonUp(button)) {
        auto [x, y] = input_.GetMousePosition();
        bus_.Emit(MouseButtonUpEvent{.button = button, .x = x, .y = y});
      }
    }

    auto [dx, dy] = input_.GetMouseDelta();
    if (dx != 0 || dy != 0) {
      auto [x, y] = input_.GetMousePosition();
      bus_.Emit(MouseMoveEvent{.x = x, .y = y, .dx = static_cast<float>(dx), .dy = static_cast<float>(dy), .screen_x = x, .screen_y = y});
    }

    float scroll = input_.GetMouseScrollDelta();
    if (scroll != 0.0f) {
      auto [x, y] = input_.GetMousePosition();
      auto [scroll_x, scroll_y] = input_.GetMouseScrollDelta2D();
      bus_.Emit(MouseWheelEvent{.delta = scroll_y, .delta_x = scroll_x, .x = x, .y = y});
    }
  }

  void GenerateGamepadEvents() {
    for (int i = 0; i < 4; ++i) {
      if (!input_.IsGamepadConnected(i)) {
        continue;
      }

      GenerateGamepadButtonEvents(i);
      GenerateGamepadAnalogEvents(i);
    }
  }

  void GenerateGamepadButtonEvents(int playerIndex) {
    auto buttons_down = input_.GetGamepadButtonsDown(playerIndex);
    for (auto button : buttons_down) {
      bus_.Emit(GamepadButtonDownEvent{.player_index = playerIndex, .button = button});
    }

    auto buttons_up = input_.GetGamepadButtonsUp(playerIndex);
    for (auto button : buttons_up) {
      bus_.Emit(GamepadButtonUpEvent{.player_index = playerIndex, .button = button});
    }
  }

  void GenerateGamepadAnalogEvents(int playerIndex) {
    auto [lx_curr, ly_curr] = input_.GetGamepadStick(Gamepad::Stick::Left, playerIndex);
    auto [lx_prev, ly_prev] = prev_gamepad_state_[playerIndex].left_stick;

    if (ShouldGenerateAxisEvent(lx_curr, lx_prev)) {
      bus_.Emit(GamepadAxisEvent{.player_index = playerIndex, .axis = GamepadAxisEvent::Axis::LeftStickX, .value = lx_curr});
    }

    if (ShouldGenerateAxisEvent(ly_curr, ly_prev)) {
      bus_.Emit(GamepadAxisEvent{.player_index = playerIndex, .axis = GamepadAxisEvent::Axis::LeftStickY, .value = ly_curr});
    }

    auto [rx_curr, ry_curr] = input_.GetGamepadStick(Gamepad::Stick::Right, playerIndex);
    auto [rx_prev, ry_prev] = prev_gamepad_state_[playerIndex].right_stick;

    if (ShouldGenerateAxisEvent(rx_curr, rx_prev)) {
      bus_.Emit(GamepadAxisEvent{.player_index = playerIndex, .axis = GamepadAxisEvent::Axis::RightStickX, .value = rx_curr});
    }

    if (ShouldGenerateAxisEvent(ry_curr, ry_prev)) {
      bus_.Emit(GamepadAxisEvent{.player_index = playerIndex, .axis = GamepadAxisEvent::Axis::RightStickY, .value = ry_curr});
    }

    float lt_curr = input_.GetGamepadTrigger(Gamepad::Trigger::Left, playerIndex);
    float lt_prev = prev_gamepad_state_[playerIndex].left_trigger;

    if (ShouldGenerateAxisEvent(lt_curr, lt_prev)) {
      bus_.Emit(GamepadTriggerEvent{.player_index = playerIndex, .trigger = Gamepad::Trigger::Left, .value = lt_curr});
    }

    float rt_curr = input_.GetGamepadTrigger(Gamepad::Trigger::Right, playerIndex);
    float rt_prev = prev_gamepad_state_[playerIndex].right_trigger;

    if (ShouldGenerateAxisEvent(rt_curr, rt_prev)) {
      bus_.Emit(GamepadTriggerEvent{.player_index = playerIndex, .trigger = Gamepad::Trigger::Right, .value = rt_curr});
    }

    prev_gamepad_state_[playerIndex].left_stick = {lx_curr, ly_curr};
    prev_gamepad_state_[playerIndex].right_stick = {rx_curr, ry_curr};
    prev_gamepad_state_[playerIndex].left_trigger = lt_curr;
    prev_gamepad_state_[playerIndex].right_trigger = rt_curr;
  }

  bool ShouldGenerateAxisEvent(float current, float previous) const {
    constexpr float DEADZONE = 0.15f;
    return std::abs(current - previous) > DEADZONE;
  }

  InputSystem& input_;
  EventBus& bus_;

  struct GamepadAnalogState {
    std::pair<float, float> left_stick = {0.0f, 0.0f};
    std::pair<float, float> right_stick = {0.0f, 0.0f};
    float left_trigger = 0.0f;
    float right_trigger = 0.0f;
  };

  GamepadAnalogState prev_gamepad_state_[4];
};
