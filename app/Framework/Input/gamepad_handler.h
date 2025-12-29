#pragma once

#include <GameInput.h>

#include <algorithm>
#include <iostream>
#include <mutex>

#include "Core/types.h"
#include "gamepad.h"

class GamepadHandler {
 public:
  static constexpr int MAX_GAMEPADS = 4;

  GamepadHandler() = default;
  ~GamepadHandler() = default;

  bool Initialize(IGameInput* gameInput) {
    HRESULT hr = gameInput->RegisterDeviceCallback(
      nullptr, GameInputKindGamepad, GameInputDeviceConnected, GameInputBlockingEnumeration, this, DeviceCallback, &device_callback_token_);

    return SUCCEEDED(hr);
  }

  void Shutdown(IGameInput* gameInput) {
    if (gameInput && device_callback_token_) {
      gameInput->UnregisterCallback(device_callback_token_, UINT64_MAX);
      device_callback_token_ = 0;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    for (int i = 0; i < MAX_GAMEPADS; ++i) {
      gamepads_[i].device.Reset();
      gamepads_[i].connected = false;
    }
  }

  void Update(IGameInput* gameInput) {
    for (int i = 0; i < MAX_GAMEPADS; ++i) {
      if (!gamepads_[i].connected || !gamepads_[i].device) {
        continue;
      }

      gamepads_[i].prev_buttons = gamepads_[i].curr_buttons;

      ComPtr<IGameInputReading> reading;
      HRESULT hr = gameInput->GetCurrentReading(GameInputKindGamepad, gamepads_[i].device.Get(), &reading);

      if (SUCCEEDED(hr) && reading) {
        GameInputGamepadState state;
        if (reading->GetGamepadState(&state)) {
          gamepads_[i].curr_buttons = state.buttons;
          gamepads_[i].left_trigger = state.leftTrigger;
          gamepads_[i].right_trigger = state.rightTrigger;
          gamepads_[i].left_stick_x = state.leftThumbstickX;
          gamepads_[i].left_stick_y = state.leftThumbstickY;
          gamepads_[i].right_stick_x = state.rightThumbstickX;
          gamepads_[i].right_stick_y = state.rightThumbstickY;
        }
      }
    }
  }

  bool GetButton(Gamepad::Button button, int playerIndex) const {
    if (!IsValidGamepad(playerIndex)) return false;
    return (gamepads_[playerIndex].curr_buttons & Gamepad::ButtonToGameInputGamepadButton(button)) != 0;
  }

  bool GetButtonDown(Gamepad::Button button, int playerIndex) const {
    if (!IsValidGamepad(playerIndex)) return false;
    auto flag = Gamepad::ButtonToGameInputGamepadButton(button);
    bool currPressed = (gamepads_[playerIndex].curr_buttons & flag) != 0;
    bool prevPressed = (gamepads_[playerIndex].prev_buttons & flag) != 0;
    return currPressed && !prevPressed;
  }

  bool GetButtonUp(Gamepad::Button button, int playerIndex) const {
    if (!IsValidGamepad(playerIndex)) return false;
    auto flag = Gamepad::ButtonToGameInputGamepadButton(button);
    bool currPressed = (gamepads_[playerIndex].curr_buttons & flag) != 0;
    bool prevPressed = (gamepads_[playerIndex].prev_buttons & flag) != 0;
    return !currPressed && prevPressed;
  }

  std::pair<float, float> GetStick(Gamepad::Stick stick, int playerIndex) const {
    if (!IsValidGamepad(playerIndex)) return {0.0f, 0.0f};
    if (stick == Gamepad::Stick::Left) {
      return {gamepads_[playerIndex].left_stick_x, gamepads_[playerIndex].left_stick_y};
    } else {
      return {gamepads_[playerIndex].right_stick_x, gamepads_[playerIndex].right_stick_y};
    }
  }

  float GetTrigger(Gamepad::Trigger trigger, int playerIndex) const {
    if (!IsValidGamepad(playerIndex)) return 0.0f;
    return (trigger == Gamepad::Trigger::Left) ? gamepads_[playerIndex].left_trigger : gamepads_[playerIndex].right_trigger;
  }

  bool IsConnected(int playerIndex) const {
    if (!Gamepad::IsValidIndex(playerIndex, MAX_GAMEPADS)) return false;
    return gamepads_[playerIndex].connected;
  }

  int GetConnectedCount() const {
    int count = 0;
    for (int i = 0; i < MAX_GAMEPADS; ++i) {
      if (gamepads_[i].connected) count++;
    }
    return count;
  }

  void SetVibration(int playerIndex, float lowFrequency, float highFrequency) {
    SetVibrationAdvanced(playerIndex, lowFrequency, highFrequency, 0.0f, 0.0f);
  }

  void SetVibrationAdvanced(int playerIndex, float lowFrequency, float highFrequency, float leftTrigger, float rightTrigger) {
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

  void StopVibration(int playerIndex) {
    SetVibration(playerIndex, 0.0f, 0.0f);
  }

 private:
  static void CALLBACK DeviceCallback(GameInputCallbackToken callbackToken,
    void* context,
    IGameInputDevice* device,
    uint64_t timestamp,
    GameInputDeviceStatus currentStatus,
    GameInputDeviceStatus previousStatus) {
    (void)callbackToken;
    (void)timestamp;
    GamepadHandler* handler = static_cast<GamepadHandler*>(context);
    handler->OnDeviceCallback(device, currentStatus, previousStatus);
  }

  void OnDeviceCallback(IGameInputDevice* device, GameInputDeviceStatus currentStatus, GameInputDeviceStatus previousStatus) {
    (void)previousStatus;
    std::lock_guard<std::mutex> lock(mutex_);

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

  bool IsValidGamepad(int index) const {
    return Gamepad::IsValidIndex(index, MAX_GAMEPADS) && gamepads_[index].connected;
  }

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
  };

  GamepadState gamepads_[MAX_GAMEPADS];
  GameInputCallbackToken device_callback_token_ = 0;
  std::mutex mutex_;
};
