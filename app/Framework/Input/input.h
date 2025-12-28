#pragma once

#include <GameInput.h>
#include <memory.h>

#include <vector>

#include "Core/types.h"
#include "keyboard.h"

class InputSystem {
 public:
  InputSystem() {
  }
  ~InputSystem() {
  }

  bool Initialize() {
    HRESULT hr = GameInputCreate(&m_gameInput);
    if (FAILED(hr)) return false;
    return true;
  }

  void Shutdown() {
  }

  void Update() {
    // Update previous keys
    memcpy(m_prevKeys, m_currKeys, sizeof(m_currKeys));
    memset(m_currKeys, 0, sizeof(m_currKeys));

    // Get current keyboard state
    ComPtr<IGameInputReading> reading;
    HRESULT hr = m_gameInput->GetCurrentReading(GameInputKindKeyboard, nullptr, &reading);

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
  }

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

 private:
  ComPtr<IGameInput> m_gameInput;

  bool m_currKeys[256] = {false};
  bool m_prevKeys[256] = {false};
};
