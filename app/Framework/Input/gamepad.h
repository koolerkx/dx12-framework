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

inline constexpr GameInputGamepadButtons ButtonToGameInputGamepadButton(Button button) noexcept {
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

inline constexpr Button GameInputGamepadButtonsToButton(GameInputGamepadButtons flag) noexcept {
  switch (flag) {
    case GameInputGamepadA:
      return Button::A;
    case GameInputGamepadB:
      return Button::B;
    case GameInputGamepadX:
      return Button::X;
    case GameInputGamepadY:
      return Button::Y;
    case GameInputGamepadLeftShoulder:
      return Button::LeftShoulder;
    case GameInputGamepadRightShoulder:
      return Button::RightShoulder;
    case GameInputGamepadView:
      return Button::View;
    case GameInputGamepadMenu:
      return Button::Menu;
    case GameInputGamepadLeftThumbstick:
      return Button::LeftThumbstick;
    case GameInputGamepadRightThumbstick:
      return Button::RightThumbstick;
    case GameInputGamepadDPadUp:
      return Button::DPadUp;
    case GameInputGamepadDPadDown:
      return Button::DPadDown;
    case GameInputGamepadDPadLeft:
      return Button::DPadLeft;
    case GameInputGamepadDPadRight:
      return Button::DPadRight;
    default:
      return Button::A;
  }
}

inline constexpr const char* ButtonToString(Button key) {
  switch (key) {
    case Button::A:
      return "A";
    case Button::B:
      return "B";
    case Button::X:
      return "X";
    case Button::Y:
      return "Y";
    case Button::LeftShoulder:
      return "LeftShoulder";
    case Button::RightShoulder:
      return "RightShoulder";
    case Button::View:
      return "View";
    case Button::Menu:
      return "Menu";
    case Button::LeftThumbstick:
      return "LeftThumbstick";
    case Button::RightThumbstick:
      return "RightThumbstick";
    case Button::DPadUp:
      return "DPadUp";
    case Button::DPadDown:
      return "DPadDown";
    case Button::DPadLeft:
      return "DPadLeft";
    case Button::DPadRight:
      return "DPadRight";
    default:
      return "Unknown";
  }
}

[[maybe_unused]] inline static constexpr GameInputGamepadButtons BUTTON_FLAGS[] = {GameInputGamepadA,
  GameInputGamepadB,
  GameInputGamepadX,
  GameInputGamepadY,
  GameInputGamepadLeftShoulder,
  GameInputGamepadRightShoulder,
  GameInputGamepadView,
  GameInputGamepadMenu,
  GameInputGamepadLeftThumbstick,
  GameInputGamepadRightThumbstick,
  GameInputGamepadDPadUp,
  GameInputGamepadDPadDown,
  GameInputGamepadDPadLeft,
  GameInputGamepadDPadRight};

inline constexpr bool IsValidIndex(int index, int maxCount = 4) noexcept {
  return index >= 0 && index < maxCount;
}
}  // namespace Gamepad
