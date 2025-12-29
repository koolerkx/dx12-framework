/**
 * @file input_events.h
 * @brief Input-specific events for the universal event system
 * @author Kooler Fan
 */

#pragma once

#include <string>

#include "Input/gamepad.h"
#include "Input/keyboard.h"
#include "Input/mouse.h"

namespace Event {

// Keyboard Events
struct KeyDownEvent {
  Keyboard::KeyCode key;
  int virtual_key;
  bool is_repeat;

  // Modifiers state at the time of event
  bool shift_pressed;
  bool ctrl_pressed;
  bool alt_pressed;
};

struct KeyUpEvent {
  Keyboard::KeyCode key;
  int virtual_key;

  bool shift_pressed;
  bool ctrl_pressed;
  bool alt_pressed;
};

// struct TextInputEvent {
//   char32_t codepoint;
//   char utf8[5];  // UTF-8 encoded character + null terminator
// };

// Mouse Events
struct MouseButtonDownEvent {
  Mouse::Button button;
  float x, y;       // Screen position
  int click_count;  // 1 = single, 2 = double click
};

struct MouseButtonUpEvent {
  Mouse::Button button;
  float x, y;
};

struct MouseMoveEvent {
  float x, y;                // Absolute position
  float dx, dy;              // Delta from last frame
  float screen_x, screen_y;  // Screen space position
};

struct MouseWheelEvent {
  float delta;    // Vertical scroll
  float delta_x;  // Horizontal scroll (for touchpad and featured mouse like logitech mx3)
  float x, y;     // Mouse position at scroll time
};

struct MouseEnterEvent {
  // Mouse entered window
};

struct MouseLeaveEvent {
  // Mouse left window
};

// Gamepad Events
struct GamepadButtonDownEvent {
  int player_index;  // 0-3
  Gamepad::Button button;
};

struct GamepadButtonUpEvent {
  int player_index;
  Gamepad::Button button;
};

struct GamepadAxisEvent {
  int player_index;
  enum class Axis { LeftStickX, LeftStickY, RightStickX, RightStickY };
  Axis axis;
  float value;  // -1.0 to 1.0
};

struct GamepadTriggerEvent {
  int player_index;
  Gamepad::Trigger trigger;
  float value;  // 0.0 to 1.0
};

struct GamepadConnectEvent {
  int player_index;
  std::string device_name;
};

struct GamepadDisconnectEvent {
  int player_index;
};

// Action Events (High-level input abstraction)

// /**
//  * ActionEvent - High-level gameplay action
//  * Maps multiple inputs to semantic game actions
//  * Example: "Jump" can be triggered by Space, Gamepad A, or Touch
//  */
// struct ActionEvent {
//   enum class Phase { Started, Performed, Canceled };

//   uint32_t action_id;       // Hash of action name
//   std::string action_name;  // "Jump", "Attack", "Pause", etc.
//   Phase phase;
//   float value;  // For analog inputs (0.0 - 1.0)

//   // Source info
//   enum class Source { Keyboard, Mouse, Gamepad, Touch, Unknown };
//   Source source;
//   int device_index;  // Which gamepad (0-3) or touch finger
// };

}  // namespace Event
