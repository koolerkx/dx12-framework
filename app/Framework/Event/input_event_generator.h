#pragma once

#include <cmath>

#include "Event/event_system.h"
#include "Input/input.h"
#include "input_events.h"

/**
 * @class InputEventGenerator
 * @brief Generates input events from InputSystem state transitions
 */
class InputEventGenerator {
 public:
  explicit InputEventGenerator(InputSystem& input) : input_(input) {
  }

  void Update() {
    GenerateKeyboardEvents();
    GenerateMouseEvents();
    GenerateGamepadEvents();
  }

 private:
  // Keyboard Events
  void GenerateKeyboardEvents() {
    // Optimized: Use GetKeysDown() instead of iterating 256 VKs
    auto keys_down = input_.GetKeysDown();
    for (auto key : keys_down) {
      int vk = Keyboard::KeyCodeToVirtualKey(key);
      Event::EventBus::Post(Event::KeyDownEvent{.key = key,
        .virtual_key = vk,
        .is_repeat = false,
        .shift_pressed = input_.GetKey(VK_SHIFT),
        .ctrl_pressed = input_.GetKey(VK_CONTROL),
        .alt_pressed = input_.GetKey(VK_MENU)});
    }

    // Optimized: Use GetKeysUp() instead of iterating 256 VKs
    auto keys_up = input_.GetKeysUp();
    for (auto key : keys_up) {
      int vk = Keyboard::KeyCodeToVirtualKey(key);
      Event::EventBus::Post(Event::KeyUpEvent{.key = key,
        .virtual_key = vk,
        .shift_pressed = input_.GetKey(VK_SHIFT),
        .ctrl_pressed = input_.GetKey(VK_CONTROL),
        .alt_pressed = input_.GetKey(VK_MENU)});
    }
  }

  // Mouse Events
  void GenerateMouseEvents() {
    // Mouse buttons
    for (int btn = 0; btn < 5; ++btn) {
      Mouse::Button button = static_cast<Mouse::Button>(btn);

      if (input_.GetMouseButtonDown(button)) {
        auto [x, y] = input_.GetMousePosition();
        Event::EventBus::Post(Event::MouseButtonDownEvent{
          .button = button,
          .x = x,
          .y = y,
          .click_count = 1  // TODO: Implement double-click detection
        });
      }

      if (input_.GetMouseButtonUp(button)) {
        auto [x, y] = input_.GetMousePosition();
        Event::EventBus::Post(Event::MouseButtonUpEvent{.button = button, .x = x, .y = y});
      }
    }

    // Mouse movement (only generate if there's actual movement)
    auto [dx, dy] = input_.GetMouseDelta();
    if (dx != 0 || dy != 0) {
      auto [x, y] = input_.GetMousePosition();
      Event::EventBus::Post(
        Event::MouseMoveEvent{.x = x, .y = y, .dx = static_cast<float>(dx), .dy = static_cast<float>(dy), .screen_x = x, .screen_y = y});
    }

    // Mouse wheel (only generate if there's actual scroll)
    float scroll = input_.GetMouseScrollDelta();
    if (scroll != 0.0f) {
      auto [x, y] = input_.GetMousePosition();
      auto [scroll_x, scroll_y] = input_.GetMouseScrollDelta2D();
      Event::EventBus::Post(Event::MouseWheelEvent{.delta = scroll_y, .delta_x = scroll_x, .x = x, .y = y});
    }
  }

  // Gamepad Events
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
    // Optimized: Use GetGamepadButtonsDown() instead of checking all buttons
    auto buttons_down = input_.GetGamepadButtonsDown(playerIndex);
    for (auto button : buttons_down) {
      Event::EventBus::Post(Event::GamepadButtonDownEvent{.player_index = playerIndex, .button = button});
    }

    // Optimized: Use GetGamepadButtonsUp() instead of checking all buttons
    auto buttons_up = input_.GetGamepadButtonsUp(playerIndex);
    for (auto button : buttons_up) {
      Event::EventBus::Post(Event::GamepadButtonUpEvent{.player_index = playerIndex, .button = button});
    }
  }

  void GenerateGamepadAnalogEvents(int playerIndex) {
    // Left stick
    auto [lx_curr, ly_curr] = input_.GetGamepadStick(Gamepad::Stick::Left, playerIndex);
    auto [lx_prev, ly_prev] = prev_gamepad_state_[playerIndex].left_stick;

    if (ShouldGenerateAxisEvent(lx_curr, lx_prev)) {
      Event::EventBus::Post(
        Event::GamepadAxisEvent{.player_index = playerIndex, .axis = Event::GamepadAxisEvent::Axis::LeftStickX, .value = lx_curr});
    }

    if (ShouldGenerateAxisEvent(ly_curr, ly_prev)) {
      Event::EventBus::Post(
        Event::GamepadAxisEvent{.player_index = playerIndex, .axis = Event::GamepadAxisEvent::Axis::LeftStickY, .value = ly_curr});
    }

    // Right stick
    auto [rx_curr, ry_curr] = input_.GetGamepadStick(Gamepad::Stick::Right, playerIndex);
    auto [rx_prev, ry_prev] = prev_gamepad_state_[playerIndex].right_stick;

    if (ShouldGenerateAxisEvent(rx_curr, rx_prev)) {
      Event::EventBus::Post(
        Event::GamepadAxisEvent{.player_index = playerIndex, .axis = Event::GamepadAxisEvent::Axis::RightStickX, .value = rx_curr});
    }

    if (ShouldGenerateAxisEvent(ry_curr, ry_prev)) {
      Event::EventBus::Post(
        Event::GamepadAxisEvent{.player_index = playerIndex, .axis = Event::GamepadAxisEvent::Axis::RightStickY, .value = ry_curr});
    }

    // Triggers
    float lt_curr = input_.GetGamepadTrigger(Gamepad::Trigger::Left, playerIndex);
    float lt_prev = prev_gamepad_state_[playerIndex].left_trigger;

    if (ShouldGenerateAxisEvent(lt_curr, lt_prev)) {
      Event::EventBus::Post(Event::GamepadTriggerEvent{.player_index = playerIndex, .trigger = Gamepad::Trigger::Left, .value = lt_curr});
    }

    float rt_curr = input_.GetGamepadTrigger(Gamepad::Trigger::Right, playerIndex);
    float rt_prev = prev_gamepad_state_[playerIndex].right_trigger;

    if (ShouldGenerateAxisEvent(rt_curr, rt_prev)) {
      Event::EventBus::Post(Event::GamepadTriggerEvent{.player_index = playerIndex, .trigger = Gamepad::Trigger::Right, .value = rt_curr});
    }

    // Update previous state for next frame
    prev_gamepad_state_[playerIndex].left_stick = {lx_curr, ly_curr};
    prev_gamepad_state_[playerIndex].right_stick = {rx_curr, ry_curr};
    prev_gamepad_state_[playerIndex].left_trigger = lt_curr;
    prev_gamepad_state_[playerIndex].right_trigger = rt_curr;
  }

  /**
   * Check if analog input changed enough to generate event (with deadzone)
   * This prevents flooding the event system with tiny analog changes
   */
  bool ShouldGenerateAxisEvent(float current, float previous) const {
    constexpr float DEADZONE = 0.15f;
    return std::abs(current - previous) > DEADZONE;
  }

  InputSystem& input_;

  // Track previous gamepad analog state for change detection
  struct GamepadAnalogState {
    std::pair<float, float> left_stick = {0.0f, 0.0f};
    std::pair<float, float> right_stick = {0.0f, 0.0f};
    float left_trigger = 0.0f;
    float right_trigger = 0.0f;
  };

  GamepadAnalogState prev_gamepad_state_[4];
};
