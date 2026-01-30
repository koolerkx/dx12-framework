#include "Application.h"

#include <memory>
#include <stdexcept>

#include "Framework/Logging/logger.h"

Application::Application(HINSTANCE__* const hInstance, const int width, const int height, const float frequency)
    : hInstance_(hInstance), hwnd_(nullptr), width_(width), height_(height), running_(true), frequency_(frequency) {
  if (!InitWindow()) {
    throw std::runtime_error("Failed to initialize window");
  }
}

Application::~Application() {
  if (hwnd_) {
    DestroyWindow(hwnd_);
  }
}

bool Application::InitWindow() {
  WNDCLASSEXW wc = {};

  wc.cbSize = sizeof(WNDCLASSEX);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance_;
  // wc.hIcon = LoadIcon(hInstance_, MAKEINTRESOURCE(IDI_APP_ICON));
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = reinterpret_cast<HBRUSH>((COLOR_WINDOW + 1));
  wc.lpszMenuName = nullptr;
  wc.lpszClassName = WINDOW_CLASS.c_str();
  wc.hIconSm = LoadIcon(hInstance_, IDI_APPLICATION);

  if (!RegisterClassExW(&wc)) {
    return false;
  }

  constexpr DWORD style = WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_THICKFRAME);

  RECT window_rect = {0, 0, static_cast<LONG>(width_), static_cast<LONG>(height_)};
  AdjustWindowRect(&window_rect, style, FALSE);

  const int window_width = window_rect.right - window_rect.left;
  const int window_height = window_rect.bottom - window_rect.top;

  const int desktop_width = GetSystemMetrics(SM_CXSCREEN);
  const int desktop_height = GetSystemMetrics(SM_CYSCREEN);
  const int window_x = (std::max)(0, (desktop_width - window_width) / 2);
  const int window_y = (std::max)(0, (desktop_height - window_height) / 2);

  hwnd_ = CreateWindowExW(0,
    WINDOW_CLASS.c_str(),
    WINDOW_NAME.c_str(),
    style,
    window_x,
    window_y,
    window_width,
    window_height,
    nullptr,
    nullptr,
    hInstance_,
    this);

  if (!hwnd_) {
    return false;
  }

  ShowWindow(hwnd_, SW_SHOW);
  UpdateWindow(hwnd_);

  timer_updater_ = std::make_unique<TimerUpdater>(60.0f);

  return true;
}

void Application::GetClientSize(UINT& width, UINT& height) const {
  RECT client_rect;
  GetClientRect(hwnd_, &client_rect);
  width = client_rect.right - client_rect.left;
  height = client_rect.bottom - client_rect.top;
}

bool Application::HandleResize(UINT new_width, UINT new_height) {
  if (resize_callback_) {
    return resize_callback_(new_width, new_height);
  }
  return true;
}

bool Application::SetBorderlessFullscreen(bool enable) {
  if (!hwnd_) {
    return false;
  }

  if (enable == is_borderless_fullscreen_) {
    return true;
  }

  if (enable) {
    // Save current window style and position
    window_style_cache_ = GetWindowLong(hwnd_, GWL_STYLE);
    GetWindowRect(hwnd_, &window_rect_cache_);

    // Get monitor info
    HMONITOR monitor = MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST);
    MONITORINFO monitor_info = {};
    monitor_info.cbSize = sizeof(MONITORINFO);

    if (!GetMonitorInfo(monitor, &monitor_info)) {
      Logger::LogFormat(LogLevel::Error, LogCategory::Core, Logger::Here(), "Failed to get monitor info");
      return false;
    }

    // Remove window decorations
    DWORD new_style = window_style_cache_ & ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
    SetWindowLong(hwnd_, GWL_STYLE, new_style);

    // Set window to cover entire monitor
    RECT& monitor_rect = monitor_info.rcMonitor;
    SetWindowPos(hwnd_,
      HWND_TOP,
      monitor_rect.left,
      monitor_rect.top,
      monitor_rect.right - monitor_rect.left,
      monitor_rect.bottom - monitor_rect.top,
      SWP_FRAMECHANGED | SWP_NOACTIVATE);

    ShowWindow(hwnd_, SW_MAXIMIZE);

    // Notify graphics system to resize
    UINT monitor_width = monitor_rect.right - monitor_rect.left;
    UINT monitor_height = monitor_rect.bottom - monitor_rect.top;

    if (!HandleResize(monitor_width, monitor_height)) {
      Logger::LogFormat(LogLevel::Error, LogCategory::Core, Logger::Here(), "Failed to resize graphics for fullscreen");
      // Try to restore window
      SetWindowLong(hwnd_, GWL_STYLE, window_style_cache_);
      SetWindowPos(hwnd_,
        HWND_NOTOPMOST,
        window_rect_cache_.left,
        window_rect_cache_.top,
        window_rect_cache_.right - window_rect_cache_.left,
        window_rect_cache_.bottom - window_rect_cache_.top,
        SWP_FRAMECHANGED | SWP_NOACTIVATE);
      return false;
    }

    is_borderless_fullscreen_ = true;

  } else {
    // Restore window style
    SetWindowLong(hwnd_, GWL_STYLE, window_style_cache_);

    // Restore window position and size
    SetWindowPos(hwnd_,
      HWND_NOTOPMOST,
      window_rect_cache_.left,
      window_rect_cache_.top,
      window_rect_cache_.right - window_rect_cache_.left,
      window_rect_cache_.bottom - window_rect_cache_.top,
      SWP_FRAMECHANGED | SWP_NOACTIVATE);

    ShowWindow(hwnd_, SW_NORMAL);

    // Notify graphics system to resize back
    RECT client_rect;
    GetClientRect(hwnd_, &client_rect);
    UINT windowed_width = client_rect.right - client_rect.left;
    UINT windowed_height = client_rect.bottom - client_rect.top;

    if (!HandleResize(windowed_width, windowed_height)) {
      Logger::LogFormat(LogLevel::Error, LogCategory::Core, Logger::Here(), "Failed to resize graphics for windowed mode");
      return false;
    }

    is_borderless_fullscreen_ = false;
  }

  return true;
}

bool Application::ToggleBorderlessFullscreen() {
  return SetBorderlessFullscreen(!is_borderless_fullscreen_);
}

LRESULT CALLBACK Application::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  Application* app = nullptr;

  if (msg == WM_CREATE) {
    const auto* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
    app = static_cast<Application*>(pCreate->lpCreateParams);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
  } else {
    app = reinterpret_cast<Application*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  }

  switch (msg) {
    case WM_DESTROY:
      if (app) {
        app->running_ = false;
      }
      return 0;

    case WM_KEYDOWN:
      if (wParam == VK_ESCAPE) {
        if (app) {
          app->running_ = false;
        }
        DestroyWindow(hwnd);
      }
      // Toggle fullscreen with F11
      if (wParam == VK_F11) {
        if (app) {
          app->ToggleBorderlessFullscreen();
        }
      }
      return 0;

    case WM_SIZE:
      if (app && wParam != SIZE_MINIMIZED) {
        UINT new_width = LOWORD(lParam);
        UINT new_height = HIWORD(lParam);
        if (new_width > 0 && new_height > 0) {
          app->HandleResize(new_width, new_height);
        }
      }
      return 0;

    default:
      break;
  }

  return DefWindowProc(hwnd, msg, wParam, lParam);
}

int Application::Run(const std::function<void(float dt)>& OnUpdate, const std::function<void(float fdt)>& OnFixedUpdate) {
  MSG msg = {};

  if (OnUpdate == nullptr || OnFixedUpdate == nullptr) {
    throw std::runtime_error("Application: OnUpdate or OnFixedUpdate is nullptr");
  }

  while (running_) {
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    timer_updater_->tick(OnUpdate, OnFixedUpdate);
  }

  return static_cast<int>(msg.wParam);
}
