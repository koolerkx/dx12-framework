#pragma once

#include "Event/event_system.h"
#include "Input/input.h"
#include "input_events.h"

class InputEventGenerator {
 public:
  explicit InputEventGenerator(InputSystem& input) : input_(input) {
  }

  void Update() {
    // Keyboard
    for (int vk = 0; vk < 256; ++vk) {
      if (input_.GetKeyDown(vk)) {
        Event::EventBus::Post(Event::KeyDownEvent{.key = Keyboard::VirtualKeyToKeyCode(vk),
          .virtual_key = vk,
          .is_repeat = false,
          .shift_pressed = input_.GetKey(VK_SHIFT),
          .ctrl_pressed = input_.GetKey(VK_CONTROL),
          .alt_pressed = input_.GetKey(VK_MENU)});
      }
      if (input_.GetKeyUp(vk)) {
        Event::EventBus::Post(Event::KeyUpEvent{.key = Keyboard::VirtualKeyToKeyCode(vk),
          .virtual_key = vk,
          .shift_pressed = input_.GetKey(VK_SHIFT),
          .ctrl_pressed = input_.GetKey(VK_CONTROL),
          .alt_pressed = input_.GetKey(VK_MENU)});
      }
    }

    // Mouse (similar pattern)
    // Gamepad (similar pattern)
  }

 private:
  InputSystem& input_;
};
