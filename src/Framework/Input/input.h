/**
 * @file input_refactored.h
 * @author Kooler Fan
 * @brief Refactored input system using delegation pattern
 */

#pragma once

#include <GameInput.h>

#include "gamepad_handler.h"
#include "keyboard_handler.h"
#include "mouse_handler.h"

class InputSystem {
 public:
  InputSystem() = default;
  ~InputSystem() = default;

  bool Initialize(HWND hwnd) {
    hwnd_ = hwnd;

    HRESULT hr = GameInputCreate(&game_input_);
    if (FAILED(hr)) return false;

    if (!gamepad_handler_.Initialize(game_input_.Get())) {
      return false;
    }

    return true;
  }

  void Shutdown() {
    gamepad_handler_.Shutdown(game_input_.Get());
  }

  void Update() {
    if (!enabled_) {
      keyboard_handler_.ClearState();
      mouse_handler_.ClearState();
      gamepad_handler_.ClearState();
      return;
    }
    keyboard_handler_.Update(game_input_.Get());
    mouse_handler_.Update(game_input_.Get(), hwnd_);
    gamepad_handler_.Update(game_input_.Get());
  }

  void SetEnabled(bool enabled) {
    enabled_ = enabled;
  }
  bool IsEnabled() const {
    return enabled_;
  }

  bool GetKey(Keyboard::KeyCode key) const {
    return keyboard_handler_.GetKey(key);
  }

  bool GetKeyDown(Keyboard::KeyCode key) const {
    return keyboard_handler_.GetKeyDown(key);
  }

  bool GetKeyUp(Keyboard::KeyCode key) const {
    return keyboard_handler_.GetKeyUp(key);
  }

  bool GetKey(int virtualKeyCode) const {
    return keyboard_handler_.GetKey(virtualKeyCode);
  }

  bool GetKeyDown(int virtualKeyCode) const {
    return keyboard_handler_.GetKeyDown(virtualKeyCode);
  }

  bool GetKeyUp(int virtualKeyCode) const {
    return keyboard_handler_.GetKeyUp(virtualKeyCode);
  }

  std::vector<Keyboard::KeyCode> GetKeys() const {
    return keyboard_handler_.GetKeys();
  }

  std::vector<Keyboard::KeyCode> GetKeysDown() const {
    return keyboard_handler_.GetKeysDown();
  }

  std::vector<Keyboard::KeyCode> GetKeysUp() const {
    return keyboard_handler_.GetKeysUp();
  }

  // Mouse methods - delegate to mouse handler
  bool GetMouseButton(Mouse::Button button) const {
    return mouse_handler_.GetButton(button);
  }

  bool GetMouseButtonDown(Mouse::Button button) const {
    return mouse_handler_.GetButtonDown(button);
  }

  bool GetMouseButtonUp(Mouse::Button button) const {
    return mouse_handler_.GetButtonUp(button);
  }

  std::pair<float, float> GetMousePosition() const {
    auto [x, y] = mouse_handler_.GetPosition();
    if (has_viewport_transform_) {
      x = (x - viewport_offset_x_) * viewport_scale_x_;
      y = (y - viewport_offset_y_) * viewport_scale_y_;
    }
    return {x, y};
  }

  void SetViewportTransform(float offset_x, float offset_y, float scale_x, float scale_y) {
    viewport_offset_x_ = offset_x;
    viewport_offset_y_ = offset_y;
    viewport_scale_x_ = scale_x;
    viewport_scale_y_ = scale_y;
    has_viewport_transform_ = true;
  }

  std::pair<int64_t, int64_t> GetMouseDelta() const {
    return mouse_handler_.GetDelta();
  }

  float GetMouseScrollDelta() const {
    return mouse_handler_.GetScrollDelta();
  }

  std::pair<float, float> GetMouseScrollDelta2D() const {
    return mouse_handler_.GetScrollDelta2D();
  }

  void SetCursorMode(Mouse::CursorMode mode) {
    mouse_handler_.SetCursorMode(mode, hwnd_);
  }

  // Gamepad methods - delegate to gamepad handler
  bool GetGamepadButton(Gamepad::Button button, int playerIndex) const {
    return gamepad_handler_.GetButton(button, playerIndex);
  }

  bool GetGamepadButtonDown(Gamepad::Button button, int playerIndex) const {
    return gamepad_handler_.GetButtonDown(button, playerIndex);
  }

  bool GetGamepadButtonUp(Gamepad::Button button, int playerIndex) const {
    return gamepad_handler_.GetButtonUp(button, playerIndex);
  }

  std::vector<Gamepad::Button> GetGamepadButtons(int playerIndex) const {
    return gamepad_handler_.GetButtons(playerIndex);
  }

  std::vector<Gamepad::Button> GetGamepadButtonsDown(int playerIndex) const {
    return gamepad_handler_.GetButtonsDown(playerIndex);
  }

  std::vector<Gamepad::Button> GetGamepadButtonsUp(int playerIndex) const {
    return gamepad_handler_.GetButtonsUp(playerIndex);
  }

  std::pair<float, float> GetGamepadStick(Gamepad::Stick stick, int playerIndex) const {
    return gamepad_handler_.GetStick(stick, playerIndex);
  }

  float GetGamepadTrigger(Gamepad::Trigger trigger, int playerIndex) const {
    return gamepad_handler_.GetTrigger(trigger, playerIndex);
  }

  bool IsGamepadConnected(int playerIndex) const {
    return gamepad_handler_.IsConnected(playerIndex);
  }

  int GetConnectedGamepadCount() const {
    return gamepad_handler_.GetConnectedCount();
  }

  void SetGamepadVibration(int playerIndex, float lowFrequency, float highFrequency) {
    gamepad_handler_.SetVibration(playerIndex, lowFrequency, highFrequency);
  }

  void SetGamepadVibrationAdvanced(int playerIndex, float lowFrequency, float highFrequency, float leftTrigger, float rightTrigger) {
    gamepad_handler_.SetVibrationAdvanced(playerIndex, lowFrequency, highFrequency, leftTrigger, rightTrigger);
  }

  void StopGamepadVibration(int playerIndex) {
    gamepad_handler_.StopVibration(playerIndex);
  }

 private:
  bool enabled_ = true;
  ComPtr<IGameInput> game_input_;
  HWND hwnd_ = nullptr;

  KeyboardHandler keyboard_handler_;
  MouseHandler mouse_handler_;
  GamepadHandler gamepad_handler_;

  bool has_viewport_transform_ = false;
  float viewport_offset_x_ = 0;
  float viewport_offset_y_ = 0;
  float viewport_scale_x_ = 1;
  float viewport_scale_y_ = 1;
};
