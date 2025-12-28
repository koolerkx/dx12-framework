// https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes

#pragma once

#include <cstdint>

namespace Keyboard {
enum class KeyCode : uint16_t {
  Unknown = 0,

  // Character
  A = 0x41,
  B = 0x42,
  C = 0x43,
  D = 0x44,
  E = 0x45,
  F = 0x46,
  G = 0x47,
  H = 0x48,
  I = 0x49,
  J = 0x4A,
  K = 0x4B,
  L = 0x4C,
  M = 0x4D,
  N = 0x4E,
  O = 0x4F,
  P = 0x50,
  Q = 0x51,
  R = 0x52,
  S = 0x53,
  T = 0x54,
  U = 0x55,
  V = 0x56,
  W = 0x57,
  X = 0x58,
  Y = 0x59,
  Z = 0x5A,

  // Number
  Num0 = 0x30,
  Num1 = 0x31,
  Num2 = 0x32,
  Num3 = 0x33,
  Num4 = 0x34,
  Num5 = 0x35,
  Num6 = 0x36,
  Num7 = 0x37,
  Num8 = 0x38,
  Num9 = 0x39,

  // Function Key
  Escape = 0x1B,  // VK_ESCAPE
  F1 = 0x70,      // VK_F1
  F2 = 0x71,      // VK_F2
  F3 = 0x72,      // VK_F3
  F4 = 0x73,      // VK_F4
  F5 = 0x74,      // VK_F5
  F6 = 0x75,      // VK_F6
  F7 = 0x76,      // VK_F7
  F8 = 0x77,      // VK_F8
  F9 = 0x78,      // VK_F9
  F10 = 0x79,     // VK_F10
  F11 = 0x7A,     // VK_F11
  F12 = 0x7B,     // VK_F12

  // Control
  Shift = 0x10,       // VK_SHIFT
  LeftShift = 0xA0,   // VK_LSHIFT
  RightShift = 0xA1,  // VK_RSHIFT
  Control = 0x11,     // VL_CONTROL
  LeftCtrl = 0xA2,    // VK_LCONTROL
  RightCtrl = 0xA3,   // VK_RCONTROL
  Alt = 0x12,         // VK_MENU
  LeftAlt = 0xA4,     // VK_LMENU
  RightAlt = 0xA5,    // VK_RMENU

  // Arrow
  Left = 0x25,   // VK_LEFT
  Up = 0x26,     // VK_UP
  Right = 0x27,  // VK_RIGHT
  Down = 0x28,   // VK_DOWN

  // Other
  Space = 0x20,      // VK_SPACE
  Enter = 0x0D,      // VK_RETURN
  Backspace = 0x08,  // VK_BACK
  Tab = 0x09,        // VK_TAB
  CapsLock = 0x14,   // VK_CAPITAL
  Insert = 0x2D,     // VK_INSERT
  DeleteKey = 0x2E,  // VK_DELETE
  Home = 0x24,       // VK_HOME
  End = 0x23,        // VK_END
  PageUp = 0x21,     // VK_PRIOR
  PageDown = 0x22,   // VK_NEXT

  // Num Pad
  Numpad0 = 0x60,          // VK_NUMPAD0
  Numpad1 = 0x61,          // VK_NUMPAD1
  Numpad2 = 0x62,          // VK_NUMPAD2
  Numpad3 = 0x63,          // VK_NUMPAD3
  Numpad4 = 0x64,          // VK_NUMPAD4
  Numpad5 = 0x65,          // VK_NUMPAD5
  Numpad6 = 0x66,          // VK_NUMPAD6
  Numpad7 = 0x67,          // VK_NUMPAD7
  Numpad8 = 0x68,          // VK_NUMPAD8
  Numpad9 = 0x69,          // VK_NUMPAD9
  NumpadMultiply = 0x6A,   // VK_MULTIPLY
  NumpadAdd = 0x6B,        // VK_ADD
  NumpadSeparator = 0x6C,  // VK_SEPARATOR
  NumpadSubtract = 0x6D,   // VK_SUBTRACT
  NumpadDecimal = 0x6E,    // VK_DECIMAL
  NumpadDivide = 0x6F,     // VK_DIVIDE
};

// Map keycode to virtual key
inline constexpr int KeyCodeToVirtualKey(KeyCode key) noexcept {
  return static_cast<int>(key);
}

// Map virtual key to keycode
inline constexpr KeyCode VirtualKeyToKeyCode(int vk) noexcept {
  if (vk < 0 || vk > 255) {
    return KeyCode::Unknown;
  }
  return static_cast<KeyCode>(vk);
}

// Map keycode to string
inline const char* KeyCodeToString(KeyCode key) {
  switch (key) {
    // Character
    case KeyCode::A:
      return "A";
    case KeyCode::B:
      return "B";
    case KeyCode::C:
      return "C";
    case KeyCode::D:
      return "D";
    case KeyCode::E:
      return "E";
    case KeyCode::F:
      return "F";
    case KeyCode::G:
      return "G";
    case KeyCode::H:
      return "H";
    case KeyCode::I:
      return "I";
    case KeyCode::J:
      return "J";
    case KeyCode::K:
      return "K";
    case KeyCode::L:
      return "L";
    case KeyCode::M:
      return "M";
    case KeyCode::N:
      return "N";
    case KeyCode::O:
      return "O";
    case KeyCode::P:
      return "P";
    case KeyCode::Q:
      return "Q";
    case KeyCode::R:
      return "R";
    case KeyCode::S:
      return "S";
    case KeyCode::T:
      return "T";
    case KeyCode::U:
      return "U";
    case KeyCode::V:
      return "V";
    case KeyCode::W:
      return "W";
    case KeyCode::X:
      return "X";
    case KeyCode::Y:
      return "Y";
    case KeyCode::Z:
      return "Z";

    // Number
    case KeyCode::Num0:
      return "Num0";
    case KeyCode::Num1:
      return "Num1";
    case KeyCode::Num2:
      return "Num2";
    case KeyCode::Num3:
      return "Num3";
    case KeyCode::Num4:
      return "Num4";
    case KeyCode::Num5:
      return "Num5";
    case KeyCode::Num6:
      return "Num6";
    case KeyCode::Num7:
      return "Num7";
    case KeyCode::Num8:
      return "Num8";
    case KeyCode::Num9:
      return "Num9";

    // Function Key
    case KeyCode::Escape:
      return "Escape";
    case KeyCode::F1:
      return "F1";
    case KeyCode::F2:
      return "F2";
    case KeyCode::F3:
      return "F3";
    case KeyCode::F4:
      return "F4";
    case KeyCode::F5:
      return "F5";
    case KeyCode::F6:
      return "F6";
    case KeyCode::F7:
      return "F7";
    case KeyCode::F8:
      return "F8";
    case KeyCode::F9:
      return "F9";
    case KeyCode::F10:
      return "F10";
    case KeyCode::F11:
      return "F11";
    case KeyCode::F12:
      return "F12";

    // Control
    case KeyCode::LeftShift:
      return "LeftShift";
    case KeyCode::RightShift:
      return "RightShift";
    case KeyCode::LeftCtrl:
      return "LeftCtrl";
    case KeyCode::RightCtrl:
      return "RightCtrl";
    case KeyCode::LeftAlt:
      return "LeftAlt";
    case KeyCode::RightAlt:
      return "RightAlt";

    // Arrow
    case KeyCode::Left:
      return "Left";
    case KeyCode::Up:
      return "Up";
    case KeyCode::Right:
      return "Right";
    case KeyCode::Down:
      return "Down";

    // Other
    case KeyCode::Space:
      return "Space";
    case KeyCode::Enter:
      return "Enter";
    case KeyCode::Backspace:
      return "Backspace";
    case KeyCode::Tab:
      return "Tab";
    case KeyCode::CapsLock:
      return "CapsLock";
    case KeyCode::Insert:
      return "Insert";
    case KeyCode::DeleteKey:
      return "DeleteKey";
    case KeyCode::Home:
      return "Home";
    case KeyCode::End:
      return "End";
    case KeyCode::PageUp:
      return "PageUp";
    case KeyCode::PageDown:
      return "PageDown";

    // Num Pad
    case KeyCode::Numpad0:
      return "Numpad0";
    case KeyCode::Numpad1:
      return "Numpad1";
    case KeyCode::Numpad2:
      return "Numpad2";
    case KeyCode::Numpad3:
      return "Numpad3";
    case KeyCode::Numpad4:
      return "Numpad4";
    case KeyCode::Numpad5:
      return "Numpad5";
    case KeyCode::Numpad6:
      return "Numpad6";
    case KeyCode::Numpad7:
      return "Numpad7";
    case KeyCode::Numpad8:
      return "Numpad8";
    case KeyCode::Numpad9:
      return "Numpad9";
    case KeyCode::NumpadMultiply:
      return "NumpadMultiply";
    case KeyCode::NumpadAdd:
      return "NumpadAdd";
    case KeyCode::NumpadSeparator:
      return "NumpadSeparator";
    case KeyCode::NumpadSubtract:
      return "NumpadSubtract";
    case KeyCode::NumpadDecimal:
      return "NumpadDecimal";
    case KeyCode::NumpadDivide:
      return "NumpadDivide";

    default:
      return "Unknown";
  }
}
}  // namespace Keyboard
