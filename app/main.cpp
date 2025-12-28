#include "Framework/Input/Keyboard.h"
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
  graphic.Initialize(app.GetHwnd(), window_width, window_height);

  Game game(graphic);
  game.Initialize();

  InputSystem inputSystem;
  (void)inputSystem.Initialize();

  const std::function<void(float dt)> OnUpdate = [&]([[maybe_unused]] float dt) {
    inputSystem.Update();

    if (inputSystem.GetKeyDown(Keyboard::KeyCode::W)) {
      std::cout << Keyboard::KeyCodeToString(Keyboard::KeyCode::W) << std::endl;
    }

    game.OnUpdate(dt);
    game.OnRender();
  };

  const std::function<void(float fdt)> OnFixedUpdate = [&]([[maybe_unused]] float fdt) { game.OnFixedUpdate(fdt); };

  app.Run(OnUpdate, OnFixedUpdate);

  inputSystem.Shutdown();
  game.Shutdown();
  graphic.Shutdown();

  return 0;
} catch (const std::exception& e) {
  std::cerr << "Exception: " << e.what() << std::endl;
  MessageBoxW(nullptr, utils::utf8_to_wstring(e.what()).c_str(), L"Error", MB_OK | MB_ICONERROR);

  return -1;
}
