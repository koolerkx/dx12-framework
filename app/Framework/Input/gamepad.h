#pragma once

#include <GameInput.h>

namespace Gamepad {
enum class Button {
  A = 0,  // GameInputGamepadA
  B = 1,  // GameInputGamepadB
  X = 2,  // GameInputGamepadX
  Y = 3,  // GameInputGamepadY

  LeftShoulder = 4,   // GameInputGamepadLeftShoulder (L1)
  RightShoulder = 5,  // GameInputGamepadRightShoulder (R1)

  View = 6,  // GameInputGamepadView (Select/Back)
  Menu = 7,  // GameInputGamepadMenu (Start)

  LeftThumbstick = 8,   // GameInputGamepadLeftThumbstick (L3)
  RightThumbstick = 9,  // GameInputGamepadRightThumbstick (R3)

  DPadUp = 10,    // GameInputGamepadDPadUp
  DPadDown = 11,  // GameInputGamepadDPadDown
  DPadLeft = 12,  // GameInputGamepadDPadLeft
  DPadRight = 13  // GameInputGamepadDPadRight
};

enum class Stick { Left, Right };

enum class Trigger {
  Left,  // L2
  Right  // R2
};

inline constexpr GameInputGamepadButtons ToFlag(Button button) noexcept {
  switch (button) {
    case Button::A:
      return GameInputGamepadA;
    case Button::B:
      return GameInputGamepadB;
    case Button::X:
      return GameInputGamepadX;
    case Button::Y:
      return GameInputGamepadY;
    case Button::LeftShoulder:
      return GameInputGamepadLeftShoulder;
    case Button::RightShoulder:
      return GameInputGamepadRightShoulder;
    case Button::View:
      return GameInputGamepadView;
    case Button::Menu:
      return GameInputGamepadMenu;
    case Button::LeftThumbstick:
      return GameInputGamepadLeftThumbstick;
    case Button::RightThumbstick:
      return GameInputGamepadRightThumbstick;
    case Button::DPadUp:
      return GameInputGamepadDPadUp;
    case Button::DPadDown:
      return GameInputGamepadDPadDown;
    case Button::DPadLeft:
      return GameInputGamepadDPadLeft;
    case Button::DPadRight:
      return GameInputGamepadDPadRight;
    default:
      return GameInputGamepadNone;
  }
}

inline constexpr bool IsValidIndex(int index, int maxCount = 4) noexcept {
  return index >= 0 && index < maxCount;
}
}  // namespace Gamepad
