#pragma once

#include "timer_updater.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <functional>
#include <memory>
#include <string>

const std::wstring WINDOW_CLASS = L"DirectX 12 Game with Engine";
const std::wstring WINDOW_NAME = L"DirectX 12 Game with Engine";

// Extract to config
constexpr int INIT_WINDOW_WIDTH = 1920;
constexpr int INIT_WINDOW_HEIGHT = 1080;

class Application {
 public:
  Application(HINSTANCE hInstance, int width = INIT_WINDOW_WIDTH, int height = INIT_WINDOW_HEIGHT, float frequency = 60.0f);
  ~Application();

  int Run(const std::function<void(float dt)>& OnUpdate = nullptr, const std::function<void(float fdt)>& OnFixedUpdate = nullptr);

  void SetFrequency(float frequency) {
    frequency_ = frequency;
  }
  [[nodiscard]] float GetFrequency() const {
    return frequency_;
  }

  void SetTimeScale(const float s) const {
    timer_updater_->SetTimeScale(s);
  }

  [[nodiscard]] HWND GetHwnd() const {
    return hwnd_;
  }

  // Fullscreen management
  bool SetBorderlessFullscreen(bool enable);
  bool IsBorderlessFullscreen() const {
    return is_borderless_fullscreen_;
  }
  bool ToggleBorderlessFullscreen();

  // Get current client area size
  void GetClientSize(UINT& width, UINT& height) const;

  // Callback for resize notification
  using ResizeCallback = std::function<bool(UINT width, UINT height)>;
  void SetResizeCallback(ResizeCallback callback) {
    resize_callback_ = callback;
  }

 private:
  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

  bool InitWindow();
  bool HandleResize(UINT new_width, UINT new_height);

  HINSTANCE hInstance_;
  HWND hwnd_;
  int width_;
  int height_;
  bool running_;
  float frequency_;

  // Fullscreen state
  bool is_borderless_fullscreen_ = false;
  DWORD window_style_cache_ = 0;
  RECT window_rect_cache_ = {};

  // Resize callback
  ResizeCallback resize_callback_;

  std::unique_ptr<TimerUpdater> timer_updater_ = nullptr;
};
