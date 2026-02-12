#pragma once

#include <GameInput.h>
#include <memory.h>

#include <vector>

#include "Core/types.h"
#include "keyboard.h"

class KeyboardHandler {
 public:
  KeyboardHandler() = default;
  ~KeyboardHandler() = default;

  void ClearState() {
    memcpy(prev_keys_, curr_keys_, sizeof(curr_keys_));
    memset(curr_keys_, 0, sizeof(curr_keys_));
    pressed_keys_.clear();
  }

  void Update(IGameInput* gameInput) {
    memcpy(prev_keys_, curr_keys_, sizeof(curr_keys_));
    memset(curr_keys_, 0, sizeof(curr_keys_));

    pressed_keys_.clear();

    ComPtr<IGameInputReading> reading;
    HRESULT hr = gameInput->GetCurrentReading(GameInputKindKeyboard, nullptr, &reading);

    if (SUCCEEDED(hr)) {
      uint32_t keyCount = reading->GetKeyCount();

      if (keyCount > 0) {
        pressed_keys_.reserve(keyCount);
        std::vector<GameInputKeyState> keyStates(keyCount);

        reading->GetKeyState(keyCount, keyStates.data());

        for (uint32_t i = 0; i < keyCount; ++i) {
          uint8_t vk = keyStates[i].virtualKey;
          curr_keys_[vk] = true;
          pressed_keys_.push_back(vk);
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

  std::vector<Keyboard::KeyCode> GetKeys() const {
    std::vector<Keyboard::KeyCode> result;
    result.reserve(pressed_keys_.size());

    for (uint8_t vk : pressed_keys_) {
      result.push_back(Keyboard::VirtualKeyToKeyCode(vk));
    }
    return result;
  }

  std::vector<Keyboard::KeyCode> GetKeysDown() const {
    std::vector<Keyboard::KeyCode> result;
    result.reserve(8);

    for (uint8_t vk : pressed_keys_) {
      if (!prev_keys_[vk]) {
        result.push_back(Keyboard::VirtualKeyToKeyCode(vk));
      }
    }
    return result;
  }

  std::vector<Keyboard::KeyCode> GetKeysUp() const {
    std::vector<Keyboard::KeyCode> result;
    result.reserve(8);

    for (int vk = 0; vk < 256; ++vk) {
      if (prev_keys_[vk] && !curr_keys_[vk]) {
        result.push_back(Keyboard::VirtualKeyToKeyCode(vk));
      }
    }
    return result;
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
  std::vector<uint8_t> pressed_keys_;
};
