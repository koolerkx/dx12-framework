#pragma once

#include <GameInput.h>
#include <Windows.h>

#include <utility>

#include "Core/types.h"
#include "mouse.h"

class MouseHandler {
 public:
  MouseHandler() = default;
  ~MouseHandler() = default;

  void Update(IGameInput* gameInput, HWND hwnd) {
    prev_mouse_buttons_ = curr_mouse_buttons_;
    curr_mouse_buttons_ = GameInputMouseNone;

    ComPtr<IGameInputReading> reading;
    HRESULT hr = gameInput->GetCurrentReading(GameInputKindMouse, nullptr, &reading);
    if (SUCCEEDED(hr) && reading) {
      GameInputMouseState mouseState;
      if (reading->GetMouseState(&mouseState)) {
        curr_mouse_buttons_ = mouseState.buttons;

        mouse_dx_ = mouseState.positionX - prev_mouse_dx_;
        mouse_dy_ = mouseState.positionY - prev_mouse_dy_;
        prev_mouse_dx_ = mouseState.positionX;
        prev_mouse_dy_ = mouseState.positionY;

        wheel_dx_ = mouseState.wheelX - prev_wheel_x_;
        wheel_dy_ = mouseState.wheelY - prev_wheel_y_;
        prev_wheel_x_ = mouseState.wheelX;
        prev_wheel_y_ = mouseState.wheelY;
      }
    }

    POINT pt;
    if (GetCursorPos(&pt) && hwnd) {
      ScreenToClient(hwnd, &pt);
      mouse_x_ = pt.x;
      mouse_y_ = pt.y;
    }

    if (cursor_mode_ == Mouse::CursorMode::Locked && hwnd) {
      RECT rect;
      GetClientRect(hwnd, &rect);
      POINT center = {(rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2};
      ClientToScreen(hwnd, &center);
      SetCursorPos(center.x, center.y);
    }
  }

  bool GetButton(Mouse::Button button) const {
    return (curr_mouse_buttons_ & Mouse::ButtonToGameInputMouseButton(button)) != 0;
  }

  bool GetButtonDown(Mouse::Button button) const {
    auto flag = Mouse::ButtonToGameInputMouseButton(button);
    return (curr_mouse_buttons_ & flag) != 0 && (prev_mouse_buttons_ & flag) == 0;
  }

  bool GetButtonUp(Mouse::Button button) const {
    auto flag = Mouse::ButtonToGameInputMouseButton(button);
    return (curr_mouse_buttons_ & flag) == 0 && (prev_mouse_buttons_ & flag) != 0;
  }

  std::pair<float, float> GetPosition() const {
    return {static_cast<float>(mouse_x_), static_cast<float>(mouse_y_)};
  }

  std::pair<int64_t, int64_t> GetDelta() const {
    return {mouse_dx_, mouse_dy_};
  }

  float GetScrollDelta() const {
    return static_cast<float>(wheel_dy_) / 120.0f;
  }

  std::pair<float, float> GetScrollDelta2D() const {
    return {static_cast<float>(wheel_dx_) / 120.0f, static_cast<float>(wheel_dy_) / 120.0f};
  }

  void SetCursorMode(Mouse::CursorMode mode, HWND hwnd) {
    cursor_mode_ = mode;

    switch (mode) {
      case Mouse::CursorMode::Normal:
        ShowCursor(TRUE);
        if (hwnd) {
          ClipCursor(nullptr);
        }
        break;

      case Mouse::CursorMode::Hidden:
        ShowCursor(FALSE);
        if (hwnd) {
          ClipCursor(nullptr);
        }
        break;

      case Mouse::CursorMode::Locked:
        ShowCursor(FALSE);
        if (hwnd) {
          RECT rect;
          GetClientRect(hwnd, &rect);
          POINT topLeft = {rect.left, rect.top};
          POINT bottomRight = {rect.right, rect.bottom};
          ClientToScreen(hwnd, &topLeft);
          ClientToScreen(hwnd, &bottomRight);
          RECT screenRect = {topLeft.x, topLeft.y, bottomRight.x, bottomRight.y};
          ClipCursor(&screenRect);
        }
        break;
    }
  }

  Mouse::CursorMode GetCursorMode() const {
    return cursor_mode_;
  }

 private:
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
};
