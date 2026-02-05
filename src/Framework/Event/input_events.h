#pragma once

#include <string>

#include "Event/event.hpp"
#include "Input/gamepad.h"
#include "Input/keyboard.h"
#include "Input/mouse.h"

struct KeyDownEvent : Event<KeyDownEvent> {
  static constexpr std::string_view EventName = "input.key_down";
  Keyboard::KeyCode key;
  int virtual_key;
  bool is_repeat;

  bool shift_pressed;
  bool ctrl_pressed;
  bool alt_pressed;
};

struct KeyUpEvent : Event<KeyUpEvent> {
  static constexpr std::string_view EventName = "input.key_up";
  Keyboard::KeyCode key;
  int virtual_key;

  bool shift_pressed;
  bool ctrl_pressed;
  bool alt_pressed;
};

struct MouseButtonDownEvent : Event<MouseButtonDownEvent> {
  static constexpr std::string_view EventName = "input.mouse_button_down";
  Mouse::Button button;
  float x, y;
  int click_count;
};

struct MouseButtonUpEvent : Event<MouseButtonUpEvent> {
  static constexpr std::string_view EventName = "input.mouse_button_up";
  Mouse::Button button;
  float x, y;
};

struct MouseMoveEvent : Event<MouseMoveEvent> {
  static constexpr std::string_view EventName = "input.mouse_move";
  float x, y;
  float dx, dy;
  float screen_x, screen_y;
};

struct MouseWheelEvent : Event<MouseWheelEvent> {
  static constexpr std::string_view EventName = "input.mouse_wheel";
  float delta;
  float delta_x;
  float x, y;
};

struct MouseEnterEvent : Event<MouseEnterEvent> {
  static constexpr std::string_view EventName = "input.mouse_enter";
};

struct MouseLeaveEvent : Event<MouseLeaveEvent> {
  static constexpr std::string_view EventName = "input.mouse_leave";
};

struct GamepadButtonDownEvent : Event<GamepadButtonDownEvent> {
  static constexpr std::string_view EventName = "input.gamepad_button_down";
  int player_index;
  Gamepad::Button button;
};

struct GamepadButtonUpEvent : Event<GamepadButtonUpEvent> {
  static constexpr std::string_view EventName = "input.gamepad_button_up";
  int player_index;
  Gamepad::Button button;
};

struct GamepadAxisEvent : Event<GamepadAxisEvent> {
  static constexpr std::string_view EventName = "input.gamepad_axis";
  int player_index;
  enum class Axis { LeftStickX, LeftStickY, RightStickX, RightStickY };
  Axis axis;
  float value;
};

struct GamepadTriggerEvent : Event<GamepadTriggerEvent> {
  static constexpr std::string_view EventName = "input.gamepad_trigger";
  int player_index;
  Gamepad::Trigger trigger;
  float value;
};

struct GamepadConnectEvent : Event<GamepadConnectEvent> {
  static constexpr std::string_view EventName = "input.gamepad_connect";
  int player_index;
  std::string device_name;
};

struct GamepadDisconnectEvent : Event<GamepadDisconnectEvent> {
  static constexpr std::string_view EventName = "input.gamepad_disconnect";
  int player_index;
};
