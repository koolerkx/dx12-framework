#include "Framework/Input/Keyboard.h"
#include "Framework/Input/Mouse.h"
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
  (void)inputSystem.Initialize(app.GetHwnd());

  const std::function<void(float dt)> OnUpdate = [&]([[maybe_unused]] float dt) {
    inputSystem.Update();

    if (inputSystem.IsGamepadConnected(0)) {
      // A 鈕跳躍
      if (inputSystem.GetGamepadButtonDown(Gamepad::Button::A, 0)) {
        std::cout << "Button A" << std::endl;
      }

      // // 左搖桿移動
      // auto [lx, ly] = inputSystem.GetGamepadStick(Gamepad::Stick::Left, 0);
      // std::cout << "Left Stick: " << lx << ", " << ly << std::endl;

      // // 右搖桿控制相機
      // auto [rx, ry] = inputSystem.GetGamepadStick(Gamepad::Stick::Right, 0);
      // std::cout << "Right Stick: " << rx << ", " << ry << std::endl;

      // L2/R2 扳機
      float leftTrigger = inputSystem.GetGamepadTrigger(Gamepad::Trigger::Left, 0);
      float rightTrigger = inputSystem.GetGamepadTrigger(Gamepad::Trigger::Right, 0);

      if (leftTrigger > 0.5f) {
        std::cout << "Left Trigger: " << leftTrigger << std::endl;
      }
      if (rightTrigger > 0.5f) {
        std::cout << "Right Trigger: " << rightTrigger << std::endl;
      }
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
