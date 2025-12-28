#pragma once

#include <GameInput.h>
#include <memory.h>

#include <vector>

#include "Core/types.h"
#include "keyboard.h"
#include "mouse.h"

class InputSystem {
 public:
  InputSystem() {
  }
  ~InputSystem() {
  }

  bool Initialize(HWND hwnd) {
    m_hwnd = hwnd;

    HRESULT hr = GameInputCreate(&m_gameInput);
    if (FAILED(hr)) return false;
    return true;
  }

  void Shutdown() {
  }

  void Update() {
    HRESULT hr = S_OK;
    ComPtr<IGameInputReading> reading;

    // Keyboard
    // Update previous keys
    memcpy(m_prevKeys, m_currKeys, sizeof(m_currKeys));
    memset(m_currKeys, 0, sizeof(m_currKeys));
    hr = m_gameInput->GetCurrentReading(GameInputKindKeyboard, nullptr, &reading);

    if (SUCCEEDED(hr)) {
      uint32_t keyCount = reading->GetKeyCount();

      std::vector<GameInputKeyState> keyStates(keyCount);

      if (keyCount > 0) {
        reading->GetKeyState(keyCount, keyStates.data());

        for (uint32_t i = 0; i < keyCount; ++i) {
          uint8_t vk = keyStates[i].virtualKey;
          m_currKeys[vk] = true;
        }

        // Mapping logic for left and right keys
        if (m_currKeys[VK_LSHIFT] || m_currKeys[VK_RSHIFT]) {
          m_currKeys[VK_SHIFT] = true;
        }
        if (m_currKeys[VK_LCONTROL] || m_currKeys[VK_RCONTROL]) {
          m_currKeys[VK_CONTROL] = true;
        }
        if (m_currKeys[VK_LMENU] || m_currKeys[VK_RMENU]) {
          m_currKeys[VK_MENU] = true;  // Alt
        }
      }
    }

    // Mouse
    m_prevMouseButtons = m_currMouseButtons;
    m_currMouseButtons = GameInputMouseNone;

    reading.Reset();
    hr = m_gameInput->GetCurrentReading(GameInputKindMouse, nullptr, &reading);
    if (SUCCEEDED(hr) && reading) {
      GameInputMouseState mouseState;
      if (reading->GetMouseState(&mouseState)) {
        m_currMouseButtons = mouseState.buttons;

        m_mouseDeltaX = mouseState.positionX - m_prevMouseDeltaX;
        m_mouseDeltaY = mouseState.positionY - m_prevMouseDeltaY;
        m_prevMouseDeltaX = mouseState.positionX;
        m_prevMouseDeltaY = mouseState.positionY;

        m_scrollDeltaX = mouseState.wheelX - m_prevWheelX;
        m_scrollDeltaY = mouseState.wheelY - m_prevWheelY;
        m_prevWheelX = mouseState.wheelX;
        m_prevWheelY = mouseState.wheelY;
      }
    }

    POINT pt;
    if (GetCursorPos(&pt) && m_hwnd) {
      ScreenToClient(m_hwnd, &pt);
      m_mouseX = pt.x;
      m_mouseY = pt.y;
    }

    if (m_cursorMode == Mouse::CursorMode::Locked && m_hwnd) {
      RECT rect;
      GetClientRect(m_hwnd, &rect);
      POINT center = {(rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2};
      ClientToScreen(m_hwnd, &center);
      SetCursorPos(center.x, center.y);
    }
  }

  // Keyboard
  bool GetKey(Keyboard::KeyCode key) const {
    return GetKey(Keyboard::KeyCodeToVirtualKey(key));
  }

  bool GetKeyDown(Keyboard::KeyCode key) const {
    return GetKeyDown(Keyboard::KeyCodeToVirtualKey(key));
  }

  bool GetKeyUp(Keyboard::KeyCode key) const {
    return GetKeyUp(Keyboard::KeyCodeToVirtualKey(key));
  }

  bool GetKey(int virtualKeyCode) const {
    if (virtualKeyCode < 0 || virtualKeyCode > 255) return false;
    return m_currKeys[virtualKeyCode];
  }

  bool GetKeyDown(int virtualKeyCode) const {
    if (virtualKeyCode < 0 || virtualKeyCode > 255) return false;
    return m_currKeys[virtualKeyCode] && !m_prevKeys[virtualKeyCode];
  }

  bool GetKeyUp(int virtualKeyCode) const {
    if (virtualKeyCode < 0 || virtualKeyCode > 255) return false;
    return !m_currKeys[virtualKeyCode] && m_prevKeys[virtualKeyCode];
  }

  // Mouse
  bool GetMouseButton(Mouse::Button button) const {
    GameInputMouseButtons flag = GameInputMouseNone;
    switch (button) {
      case Mouse::Button::Left:
        flag = GameInputMouseLeftButton;
        break;
      case Mouse::Button::Right:
        flag = GameInputMouseRightButton;
        break;
      case Mouse::Button::Middle:
        flag = GameInputMouseMiddleButton;
        break;
      case Mouse::Button::Button4:
        flag = GameInputMouseButton4;
        break;
      case Mouse::Button::Button5:
        flag = GameInputMouseButton5;
        break;
    }
    return (m_currMouseButtons & flag) != 0;
  };

  bool GetMouseButtonDown(Mouse::Button button) const {
    GameInputMouseButtons flag = GameInputMouseNone;
    switch (button) {
      case Mouse::Button::Left:
        flag = GameInputMouseLeftButton;
        break;
      case Mouse::Button::Right:
        flag = GameInputMouseRightButton;
        break;
      case Mouse::Button::Middle:
        flag = GameInputMouseMiddleButton;
        break;
      case Mouse::Button::Button4:
        flag = GameInputMouseButton4;
        break;
      case Mouse::Button::Button5:
        flag = GameInputMouseButton5;
        break;
    }
    return (m_currMouseButtons & flag) != 0 && (m_prevMouseButtons & flag) == 0;
  }

  bool GetMouseButtonUp(Mouse::Button button) const {
    GameInputMouseButtons flag = GameInputMouseNone;
    switch (button) {
      case Mouse::Button::Left:
        flag = GameInputMouseLeftButton;
        break;
      case Mouse::Button::Right:
        flag = GameInputMouseRightButton;
        break;
      case Mouse::Button::Middle:
        flag = GameInputMouseMiddleButton;
        break;
      case Mouse::Button::Button4:
        flag = GameInputMouseButton4;
        break;
      case Mouse::Button::Button5:
        flag = GameInputMouseButton5;
        break;
    }
    return (m_currMouseButtons & flag) == 0 && (m_prevMouseButtons & flag) != 0;
  }

  std::pair<float, float> GetMousePosition() const {
    return {static_cast<float>(m_mouseX), static_cast<float>(m_mouseY)};
  }

  std::pair<int64_t, int64_t> GetMouseDelta() const {
    return {m_mouseDeltaX, m_mouseDeltaY};
  }

  void SetCursorMode(Mouse::CursorMode mode) {
    m_cursorMode = mode;

    switch (mode) {
      case Mouse::CursorMode::Normal:
        ShowCursor(TRUE);
        if (m_hwnd) {
          ClipCursor(nullptr);  // 解除游標限制
        }
        break;

      case Mouse::CursorMode::Hidden:
        ShowCursor(FALSE);
        if (m_hwnd) {
          ClipCursor(nullptr);
        }
        break;

      case Mouse::CursorMode::Locked:
        ShowCursor(FALSE);
        if (m_hwnd) {
          RECT rect;
          GetClientRect(m_hwnd, &rect);
          POINT topLeft = {rect.left, rect.top};
          POINT bottomRight = {rect.right, rect.bottom};
          ClientToScreen(m_hwnd, &topLeft);
          ClientToScreen(m_hwnd, &bottomRight);
          RECT screenRect = {topLeft.x, topLeft.y, bottomRight.x, bottomRight.y};
          ClipCursor(&screenRect);  // 限制游標在視窗內
        }
        break;
    }
  }

  float GetMouseScrollDelta() const {
    return static_cast<float>(m_scrollDeltaY) / 120.0f;
  }

  std::pair<float, float> GetMouseScrollDelta2D() const {
    return {static_cast<float>(m_scrollDeltaX) / 120.0f, static_cast<float>(m_scrollDeltaY) / 120.0f};
  }

 private:
  ComPtr<IGameInput> m_gameInput;

  // Keyboard
  bool m_currKeys[256] = {false};
  bool m_prevKeys[256] = {false};

  GameInputMouseButtons m_currMouseButtons = GameInputMouseNone;
  GameInputMouseButtons m_prevMouseButtons = GameInputMouseNone;

  int64_t m_mouseX = 0;
  int64_t m_mouseY = 0;

  int64_t m_mouseDeltaX = 0;
  int64_t m_mouseDeltaY = 0;
  int64_t m_prevMouseDeltaX = 0;
  int64_t m_prevMouseDeltaY = 0;

  int64_t m_prevWheelX = 0;
  int64_t m_prevWheelY = 0;
  int64_t m_scrollDeltaX = 0;
  int64_t m_scrollDeltaY = 0;

  Mouse::CursorMode m_cursorMode = Mouse::CursorMode::Normal;
  HWND m_hwnd = nullptr;
};
