#include "Framework/Event/event_system.h"
#include "Framework/Event/input_event_generator.h"
#include "Framework/Event/input_events.h"
#include "Framework/Input/GamePad.h"
#include "Framework/Input/Keyboard.h"
#include "Framework/Input/Mouse.h"
#include "Framework/Input/gamepad.h"
#define WIN32_LEAN_AND_MEAN
#include <DirectXMath.h>
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <exception>
#include <iostream>

#include "Application/Application.h"
#include "Core/utils.h"
#include "Framework/Input/input.h"
#include "Game/game.h"
#include "Graphic/graphic.h"

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

using namespace DirectX;

constexpr int window_width = 1920;
constexpr int window_height = 1080;

int WINAPI wWinMain([[maybe_unused]] HINSTANCE hInstance,
  [[maybe_unused]] HINSTANCE hPrevInstance,
  [[maybe_unused]] PWSTR lpCmdLine,
  [[maybe_unused]] int nCmdShow) try {
  (void)CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  (void)SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

  Application app(hInstance, window_width, window_height);
  Graphic graphic;

  GraphicInitProps graphic_props{.enable_vsync = true};
  if (!graphic.Initialize(app.GetHwnd(), window_width, window_height, graphic_props)) {
    throw std::runtime_error("Failed to initialize graphics system");
  }

  Game game(graphic);
  game.Initialize();

  InputSystem inputSystem;
  if (!inputSystem.Initialize(app.GetHwnd())) {
    throw std::runtime_error("Failed to initialize input system");
  }

  InputEventGenerator event_generator(inputSystem);

  const std::function<void(float dt)> OnUpdate = [&]([[maybe_unused]] float dt) {
    inputSystem.Update();
    event_generator.Update();

    Event::EventBus::Flush();

    game.OnUpdate(dt);
    game.OnRender();
  };

  const std::function<void(float fdt)> OnFixedUpdate = [&]([[maybe_unused]] float fdt) { game.OnFixedUpdate(fdt); };

  app.Run(OnUpdate, OnFixedUpdate);

  // Cleanup should be call explicitly
  Event::EventBus::Clear();
  inputSystem.Shutdown();
  game.Shutdown();
  graphic.Shutdown();

  return 0;
} catch (const std::system_error& e) {
  std::string error_msg = std::string("System error: ") + e.what() + " (code: " + std::to_string(e.code().value()) + ")";
  std::cerr << error_msg << std::endl;

  // Showing message box in deadlog exception casue freeze
  if (e.code().value() == static_cast<int>(std::errc::resource_deadlock_would_occur)) {
    return -1;
  }

  MessageBoxA(nullptr, error_msg.c_str(), "System Error", MB_OK | MB_ICONERROR);

  return -1;
} catch (const std::exception& e) {
  std::cerr << "Fatal exception: " << e.what() << std::endl;
  MessageBoxW(nullptr, utils::utf8_to_wstring(e.what()).c_str(), L"Fatal Error", MB_OK | MB_ICONERROR);
  return -1;
}
