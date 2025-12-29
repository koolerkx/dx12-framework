#pragma once

#include <GameInput.h>
#include <memory.h>

#include <vector>

#include "keyboard.h"
#include "Core/types.h"

class KeyboardHandler {
 public:
  KeyboardHandler() = default;
  ~KeyboardHandler() = default;

  void Update(IGameInput* gameInput) {
    memcpy(prev_keys_, curr_keys_, sizeof(curr_keys_));
    memset(curr_keys_, 0, sizeof(curr_keys_));

    ComPtr<IGameInputReading> reading;
    HRESULT hr = gameInput->GetCurrentReading(GameInputKindKeyboard, nullptr, &reading);

    if (SUCCEEDED(hr)) {
      uint32_t keyCount = reading->GetKeyCount();
      std::vector<GameInputKeyState> keyStates(keyCount);

      if (keyCount > 0) {
        reading->GetKeyState(keyCount, keyStates.data());

        for (uint32_t i = 0; i < keyCount; ++i) {
          uint8_t vk = keyStates[i].virtualKey;
          curr_keys_[vk] = true;
        }

        Keyboard::MergeModifierKeys(curr_keys_);
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
    return curr_keys_[virtualKeyCode];
  }

  bool GetKeyDown(int virtualKeyCode) const {
    if (virtualKeyCode < 0 || virtualKeyCode > 255) return false;
    return curr_keys_[virtualKeyCode] && !prev_keys_[virtualKeyCode];
  }

  bool GetKeyUp(int virtualKeyCode) const {
    if (virtualKeyCode < 0 || virtualKeyCode > 255) return false;
    return !curr_keys_[virtualKeyCode] && prev_keys_[virtualKeyCode];
  }

 private:
  bool curr_keys_[256] = {false};
  bool prev_keys_[256] = {false};
};
