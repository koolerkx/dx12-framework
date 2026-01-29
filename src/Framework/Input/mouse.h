#pragma once

#include <GameInput.h>

namespace Mouse {
enum class Button { Left = 0, Right = 1, Middle = 2, Button4 = 3, Button5 = 4 };

enum class CursorMode { Normal, Hidden, Locked };

inline constexpr const char* ButtonToString(Button button) {
  switch (button) {
    case Button::Left:
      return "Left";
    case Button::Right:
      return "Right";
    case Button::Middle:
      return "Middle";
    case Button::Button4:
      return "Button4";
    case Button::Button5:
      return "Button5";
    default:
      return "Unknown";
  }
}

inline constexpr GameInputMouseButtons ButtonToGameInputMouseButton(Button button) noexcept {
  switch (button) {
    case Button::Left:
      return GameInputMouseLeftButton;
    case Button::Right:
      return GameInputMouseRightButton;
    case Button::Middle:
      return GameInputMouseMiddleButton;
    case Button::Button4:
      return GameInputMouseButton4;
    case Button::Button5:
      return GameInputMouseButton5;
    default:
      return GameInputMouseNone;
  }
}

}  // namespace Mouse
