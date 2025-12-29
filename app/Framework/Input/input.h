/**
 * @file input.h
 * @author Kooler Fan
 * @brief Read input from keyboard, mouse and gamepad using Microsoft GameInput
 * @ref https://learn.microsoft.com/en-us/gaming/gdk/docs/features/common/input/gc-input-toc
 */

#pragma once

#include <GameInput.h>
#include <memory.h>

#include <algorithm>
#include <iostream>
#include <mutex>
#include <vector>

#include "Core/types.h"
#include "gamepad.h"
#include "keyboard.h"
#include "mouse.h"

class InputSystem {
 public:
  InputSystem() {
  }
  ~InputSystem() {
  }

  bool Initialize(HWND hwnd) {
    hwnd_ = hwnd;
    HRESULT hr = GameInputCreate(&game_input_);
    if (FAILED(hr)) return false;

    hr = game_input_->RegisterDeviceCallback(
      nullptr, GameInputKindGamepad, GameInputDeviceConnected, GameInputBlockingEnumeration, this, DeviceCallback, &device_callback_token_);

    if (FAILED(hr)) {
      return false;
    }

    return true;
  }

  void Shutdown() {
    if (game_input_ && device_callback_token_) {
      game_input_->UnregisterCallback(device_callback_token_, UINT64_MAX);
      device_callback_token_ = 0;
    }

    {
      std::lock_guard<std::mutex> lock(gamepad_mutex_);
      for (int i = 0; i < MAX_GAMEPADS; ++i) {
        gamepads_[i].device.Reset();
        gamepads_[i].connected = false;
      }
    }
  }

  static void CALLBACK DeviceCallback(GameInputCallbackToken callbackToken,
    void* context,
    IGameInputDevice* device,
    uint64_t timestamp,
    GameInputDeviceStatus currentStatus,
    GameInputDeviceStatus previousStatus) {
    (void)callbackToken;
    (void)timestamp;
    InputSystem* inputSystem = static_cast<InputSystem*>(context);
    inputSystem->OnDeviceCallback(device, currentStatus, previousStatus);
  }

  void OnDeviceCallback(IGameInputDevice* device, GameInputDeviceStatus currentStatus, GameInputDeviceStatus previousStatus) {
    (void)previousStatus;
    std::lock_guard<std::mutex> lock(gamepad_mutex_);

    bool isConnected = (currentStatus & GameInputDeviceConnected) != 0;

    if (isConnected) {
      for (int i = 0; i < MAX_GAMEPADS; ++i) {
        if (!gamepads_[i].connected) {
          gamepads_[i] = GamepadState{};
          gamepads_[i].device = device;
          gamepads_[i].connected = true;
          std::cout << "Gamepad " << i << " connected" << std::endl;
          break;
        }
      }
    } else {
      for (int i = 0; i < MAX_GAMEPADS; ++i) {
        if (gamepads_[i].device.Get() == device) {
          gamepads_[i].device.Reset();
          gamepads_[i].connected = false;
          std::cout << "Gamepad " << i << " disconnected" << std::endl;
          break;
        }
      }
    }
  }

  void Update() {
    ComPtr<IGameInputReading> reading;

    UpdateKeyboardState(reading);
    UpdateMouseState(reading);
    UpdateGamepadState();
  }

  void UpdateKeyboardState(ComPtr<IGameInputReading> reading) {
    memcpy(keyboard_state_.prev_keys_, keyboard_state_.curr_keys_, sizeof(keyboard_state_.curr_keys_));
    memset(keyboard_state_.curr_keys_, 0, sizeof(keyboard_state_.curr_keys_));

    reading.Reset();
    HRESULT hr = game_input_->GetCurrentReading(GameInputKindKeyboard, nullptr, &reading);

    if (SUCCEEDED(hr)) {
      uint32_t keyCount = reading->GetKeyCount();

      std::vector<GameInputKeyState> keyStates(keyCount);

      if (keyCount > 0) {
        reading->GetKeyState(keyCount, keyStates.data());

        for (uint32_t i = 0; i < keyCount; ++i) {
          uint8_t vk = keyStates[i].virtualKey;
          keyboard_state_.curr_keys_[vk] = true;
        }

        Keyboard::MergeModifierKeys(keyboard_state_.curr_keys_);
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
    return keyboard_state_.curr_keys_[virtualKeyCode];
  }

  bool GetKeyDown(int virtualKeyCode) const {
    if (virtualKeyCode < 0 || virtualKeyCode > 255) return false;
    return keyboard_state_.curr_keys_[virtualKeyCode] && !keyboard_state_.prev_keys_[virtualKeyCode];
  }

  bool GetKeyUp(int virtualKeyCode) const {
    if (virtualKeyCode < 0 || virtualKeyCode > 255) return false;
    return !keyboard_state_.curr_keys_[virtualKeyCode] && keyboard_state_.prev_keys_[virtualKeyCode];
  }

  // Mouse
  void UpdateMouseState(ComPtr<IGameInputReading> reading) {
    mouse_state_.prev_mouse_buttons_ = mouse_state_.curr_mouse_buttons_;
    mouse_state_.curr_mouse_buttons_ = GameInputMouseNone;

    reading.Reset();
    HRESULT hr = game_input_->GetCurrentReading(GameInputKindMouse, nullptr, &reading);
    if (SUCCEEDED(hr) && reading) {
      GameInputMouseState mouseState;
      if (reading->GetMouseState(&mouseState)) {
        mouse_state_.curr_mouse_buttons_ = mouseState.buttons;

        mouse_state_.mouse_dx_ = mouseState.positionX - mouse_state_.prev_mouse_dx_;
        mouse_state_.mouse_dy_ = mouseState.positionY - mouse_state_.prev_mouse_dy_;
        mouse_state_.prev_mouse_dx_ = mouseState.positionX;
        mouse_state_.prev_mouse_dy_ = mouseState.positionY;

        mouse_state_.wheel_dx_ = mouseState.wheelX - mouse_state_.prev_wheel_x_;
        mouse_state_.wheel_dy_ = mouseState.wheelY - mouse_state_.prev_wheel_y_;
        mouse_state_.prev_wheel_x_ = mouseState.wheelX;
        mouse_state_.prev_wheel_y_ = mouseState.wheelY;
      }
    }

    POINT pt;
    if (GetCursorPos(&pt) && hwnd_) {
      ScreenToClient(hwnd_, &pt);
      mouse_state_.mouse_x_ = pt.x;
      mouse_state_.mouse_y_ = pt.y;
    }

    if (mouse_state_.cursor_mode_ == Mouse::CursorMode::Locked && hwnd_) {
      RECT rect;
      GetClientRect(hwnd_, &rect);
      POINT center = {(rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2};
      ClientToScreen(hwnd_, &center);
      SetCursorPos(center.x, center.y);
    }
  }

  bool GetMouseButton(Mouse::Button button) const {
    return (mouse_state_.curr_mouse_buttons_ & Mouse::ButtonToGameInputMouseButton(button)) != 0;
  }

  bool GetMouseButtonDown(Mouse::Button button) const {
    auto flag = Mouse::ButtonToGameInputMouseButton(button);
    return (mouse_state_.curr_mouse_buttons_ & flag) != 0 && (mouse_state_.prev_mouse_buttons_ & flag) == 0;
  }

  bool GetMouseButtonUp(Mouse::Button button) const {
    auto flag = Mouse::ButtonToGameInputMouseButton(button);
    return (mouse_state_.curr_mouse_buttons_ & flag) == 0 && (mouse_state_.prev_mouse_buttons_ & flag) != 0;
  }

  std::pair<float, float> GetMousePosition() const {
    return {static_cast<float>(mouse_state_.mouse_x_), static_cast<float>(mouse_state_.mouse_y_)};
  }

  std::pair<int64_t, int64_t> GetMouseDelta() const {
    return {mouse_state_.mouse_dx_, mouse_state_.mouse_dy_};
  }

  float GetMouseScrollDelta() const {
    return static_cast<float>(mouse_state_.wheel_dy_) / 120.0f;
  }

  std::pair<float, float> GetMouseScrollDelta2D() const {
    return {static_cast<float>(mouse_state_.wheel_dx_) / 120.0f, static_cast<float>(mouse_state_.wheel_dy_) / 120.0f};
  }

  void SetCursorMode(Mouse::CursorMode mode) {
    mouse_state_.cursor_mode_ = mode;

    switch (mode) {
      case Mouse::CursorMode::Normal:
        ShowCursor(TRUE);
        if (hwnd_) {
          ClipCursor(nullptr);
        }
        break;

      case Mouse::CursorMode::Hidden:
        ShowCursor(FALSE);
        if (hwnd_) {
          ClipCursor(nullptr);
        }
        break;

      case Mouse::CursorMode::Locked:
        ShowCursor(FALSE);
        if (hwnd_) {
          RECT rect;
          GetClientRect(hwnd_, &rect);
          POINT topLeft = {rect.left, rect.top};
          POINT bottomRight = {rect.right, rect.bottom};
          ClientToScreen(hwnd_, &topLeft);
          ClientToScreen(hwnd_, &bottomRight);
          RECT screenRect = {topLeft.x, topLeft.y, bottomRight.x, bottomRight.y};
          ClipCursor(&screenRect);
        }
        break;
    }
  }

  // GamePad
  void UpdateGamepadState() {
    for (int i = 0; i < MAX_GAMEPADS; ++i) {
      if (!gamepads_[i].connected || !gamepads_[i].device) {
        continue;
      }

      gamepads_[i].prev_buttons = gamepads_[i].curr_buttons;

      ComPtr<IGameInputReading> gamepadReading;
      HRESULT gamepadHr = game_input_->GetCurrentReading(GameInputKindGamepad, gamepads_[i].device.Get(), &gamepadReading);

      if (SUCCEEDED(gamepadHr) && gamepadReading) {
        GameInputGamepadState gamepadState;
        if (gamepadReading->GetGamepadState(&gamepadState)) {
          gamepads_[i].curr_buttons = gamepadState.buttons;
          gamepads_[i].left_trigger = gamepadState.leftTrigger;
          gamepads_[i].right_trigger = gamepadState.rightTrigger;
          gamepads_[i].left_stick_x = gamepadState.leftThumbstickX;
          gamepads_[i].left_stick_y = gamepadState.leftThumbstickY;
          gamepads_[i].right_stick_x = gamepadState.rightThumbstickX;
          gamepads_[i].right_stick_y = gamepadState.rightThumbstickY;
        }
      }
    }
  }

  bool GetGamepadButton(Gamepad::Button button, int playerIndex) const {
    if (!IsGamepadValid(playerIndex)) return false;
    return (gamepads_[playerIndex].curr_buttons & Gamepad::ButtonToGameInputGamepadButton(button)) != 0;
  }

  bool GetGamepadButtonDown(Gamepad::Button button, int playerIndex) const {
    if (!IsGamepadValid(playerIndex)) return false;
    auto flag = Gamepad::ButtonToGameInputGamepadButton(button);
    bool currPressed = (gamepads_[playerIndex].curr_buttons & flag) != 0;
    bool prevPressed = (gamepads_[playerIndex].prev_buttons & flag) != 0;
    return currPressed && !prevPressed;
  }

  bool GetGamepadButtonUp(Gamepad::Button button, int playerIndex) const {
    if (!IsGamepadValid(playerIndex)) return false;
    auto flag = Gamepad::ButtonToGameInputGamepadButton(button);
    bool currPressed = (gamepads_[playerIndex].curr_buttons & flag) != 0;
    bool prevPressed = (gamepads_[playerIndex].prev_buttons & flag) != 0;
    return !currPressed && prevPressed;
  }

  std::pair<float, float> GetGamepadStick(Gamepad::Stick stick, int playerIndex) const {
    if (!IsGamepadValid(playerIndex)) return {0.0f, 0.0f};
    if (stick == Gamepad::Stick::Left) {
      return {gamepads_[playerIndex].left_stick_x, gamepads_[playerIndex].left_stick_y};
    } else {
      return {gamepads_[playerIndex].right_stick_x, gamepads_[playerIndex].right_stick_y};
    }
  }

  float GetGamepadTrigger(Gamepad::Trigger trigger, int playerIndex) const {
    if (!IsGamepadValid(playerIndex)) return 0.0f;
    return (trigger == Gamepad::Trigger::Left) ? gamepads_[playerIndex].left_trigger : gamepads_[playerIndex].right_trigger;
  }

  bool IsGamepadConnected(int playerIndex) const {
    if (!Gamepad::IsValidIndex(playerIndex, MAX_GAMEPADS)) return false;
    return gamepads_[playerIndex].connected;
  }

  int GetConnectedGamepadCount() const {
    int count = 0;
    for (int i = 0; i < MAX_GAMEPADS; ++i) {
      if (gamepads_[i].connected) count++;
    }
    return count;
  }

  void SetGamepadVibration(int playerIndex, float lowFrequency, float highFrequency) {
    SetGamepadVibrationAdvanced(playerIndex, lowFrequency, highFrequency, 0.0f, 0.0f);
  }

  void SetGamepadVibrationAdvanced(int playerIndex, float lowFrequency, float highFrequency, float leftTrigger, float rightTrigger) {
    if (playerIndex < 0 || playerIndex >= MAX_GAMEPADS) return;

    auto& pad = gamepads_[playerIndex];
    if (!pad.connected || !pad.device) return;

    GameInputRumbleParams rumble = {};
    rumble.lowFrequency = std::clamp(lowFrequency, 0.0f, 1.0f);
    rumble.highFrequency = std::clamp(highFrequency, 0.0f, 1.0f);
    rumble.leftTrigger = std::clamp(leftTrigger, 0.0f, 1.0f);
    rumble.rightTrigger = std::clamp(rightTrigger, 0.0f, 1.0f);

    pad.device->SetRumbleState(&rumble);
  }

  void StopGamepadVibration(int playerIndex) {
    SetGamepadVibration(playerIndex, 0.0f, 0.0f);
  }

 private:
  bool IsGamepadValid(int index) const {
    return Gamepad::IsValidIndex(index, MAX_GAMEPADS) && gamepads_[index].connected;
  }

  ComPtr<IGameInput> game_input_;

  // Keyboard
  struct KeyboardState {
    bool curr_keys_[256] = {false};
    bool prev_keys_[256] = {false};
  } keyboard_state_;

  // Mouse
  struct MouseState {
    GameInputMouseButtons curr_mouse_buttons_ = GameInputMouseNone;
    GameInputMouseButtons prev_mouse_buttons_ = GameInputMouseNone;

    int64_t mouse_x_ = 0;
    int64_t mouse_y_ = 0;

    int64_t mouse_dx_ = 0;
    int64_t mouse_dy_ = 0;
    int64_t prev_mouse_dx_ = 0;
    int64_t prev_mouse_dy_ = 0;

    int64_t prev_wheel_x_ = 0;
    int64_t prev_wheel_y_ = 0;
    int64_t wheel_dx_ = 0;
    int64_t wheel_dy_ = 0;

    Mouse::CursorMode cursor_mode_ = Mouse::CursorMode::Normal;
  } mouse_state_;
  HWND hwnd_ = nullptr;

  // GamePad
  static constexpr int MAX_GAMEPADS = 4;

  struct GamepadState {
    ComPtr<IGameInputDevice> device;
    GameInputGamepadButtons curr_buttons = GameInputGamepadNone;
    GameInputGamepadButtons prev_buttons = GameInputGamepadNone;
    float left_trigger = 0.0f;
    float right_trigger = 0.0f;
    float left_stick_x = 0.0f;
    float left_stick_y = 0.0f;
    float right_stick_x = 0.0f;
    float right_stick_y = 0.0f;
    bool connected = false;
  } gamepads_[MAX_GAMEPADS];

  GameInputCallbackToken device_callback_token_ = 0;
  std::mutex gamepad_mutex_;
};
