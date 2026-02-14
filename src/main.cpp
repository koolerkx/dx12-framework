#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <exception>

#include "Application/Application.h"
#ifdef ENABLE_EDITOR
#include "Editor/editor_layer.h"
#endif
#include "Core/utils.h"
#include "Framework/Event/event_bus.hpp"
#include "Framework/Event/input_event_generator.h"
#include "Framework/Event/thread_pool.hpp"
#include "Framework/Input/input.h"
#include "Framework/Logging/logger.h"
#include "Framework/Logging/sinks.h"
#include "Application/config_loader.h"
#include "Game/game.h"
#include "Game/game_context.h"
#include "Graphic/graphic.h"

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

void InitializeLogger();

int WINAPI wWinMain([[maybe_unused]] HINSTANCE hInstance,
  [[maybe_unused]] HINSTANCE hPrevInstance,
  [[maybe_unused]] PWSTR lpCmdLine,
  [[maybe_unused]] int nCmdShow) try {
  (void)CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  (void)SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

  // Initialize Logger (before any other systems)
  InitializeLogger();
  Logger::LogFormat(LogLevel::Info, LogCategory::Core, Logger::Here(), "Application starting...");

  AppConfig config = ConfigLoader::LoadFromFile("Content/app_config.yaml");

  Application app(hInstance, config.window_width, config.window_height);
  Graphic graphic;

  Graphic::GraphicInitProps graphic_props{
      .enable_vsync = config.vsync,
      .bloom = config.bloom,
      .ssao = config.ssao,
      .smaa = config.smaa,
  };
  if (!graphic.Initialize(app.GetHwnd(), config.window_width, config.window_height, graphic_props)) {
    throw std::runtime_error("Failed to initialize graphics system");
  }

  app.SetResizeCallback([&graphic](UINT width, UINT height) -> bool { return graphic.ResizeBuffers(width, height); });

#ifdef ENABLE_EDITOR
  EditorLayer editor;
  editor.Initialize(app.GetHwnd(), graphic);
  app.SetWndProcHook([&editor](HWND h, UINT m, WPARAM w, LPARAM l) { return editor.WndProcHandler(h, m, w, l); });
  app.SetFullscreenCallback([](bool is_fullscreen) {
    ImGui::GetIO().IniFilename = is_fullscreen ? "imgui_fullscreen.ini" : "imgui.ini";
  });
  graphic.SetOverlayRenderer([&editor](ID3D12GraphicsCommandList* cmd) { editor.Render(cmd); });
#endif

  InputSystem inputSystem;
  if (!inputSystem.Initialize(app.GetHwnd())) {
    throw std::runtime_error("Failed to initialize input system");
  }

  ThreadPool thread_pool;
  auto event_bus = std::make_shared<EventBus>(thread_pool);

  GameContext context;
  context.SetGraphic(&graphic);
  context.SetInputSystem(&inputSystem);
  context.SetEventBus(event_bus);

  Game game;
  game.Initialize({
    .context = &context,
#ifndef ENABLE_EDITOR
    .auto_play = true,
#endif
    .scene_defaults = config.scene_defaults,
  });

#ifdef ENABLE_EDITOR
  editor.SubscribeEvents(*event_bus);
  editor.SetGame(&game);
  editor.SetScene(game.GetCurrentScene());
  editor.SetDebugDrawer(context.GetDebugDrawer());
#endif

  InputEventGenerator event_generator(inputSystem, *event_bus);

  const std::function<void(float dt)> OnUpdate = [&]([[maybe_unused]] float dt) {
#ifdef ENABLE_EDITOR
    editor.BeginFrame();
#endif

    inputSystem.Update();
    event_generator.Update();

    game.OnUpdate(dt);
    game.OnRender();
  };

  const std::function<void(float fdt)> OnFixedUpdate = [&]([[maybe_unused]] float fdt) { game.OnFixedUpdate(fdt); };

  app.Run(OnUpdate, OnFixedUpdate);

#ifdef ENABLE_EDITOR
  graphic.SetOverlayRenderer(nullptr);
  editor.Shutdown(graphic);
#endif

  inputSystem.Shutdown();
  game.Shutdown();
  graphic.Shutdown();

  Logger::LogFormat(LogLevel::Info, LogCategory::Core, Logger::Here(), "Application shutdown complete");
  Logger::Shutdown();

  return 0;
} catch (const std::system_error& e) {
  Logger::LogFormat(LogLevel::Error, LogCategory::Core, Logger::Here(), "System error: {} (code: {})", e.what(), e.code().value());

  // Showing message box in deadlog exception casue freeze
  if (e.code().value() == static_cast<int>(std::errc::resource_deadlock_would_occur)) {
    Logger::Shutdown();
    return -1;
  }

  std::string error_msg = std::string("System error: ") + e.what() + " (code: " + std::to_string(e.code().value()) + ")";
  MessageBoxA(nullptr, error_msg.c_str(), "System Error", MB_OK | MB_ICONERROR);
  Logger::Shutdown();

  return -1;
} catch (const std::exception& e) {
  Logger::LogFormat(LogLevel::Error, LogCategory::Core, Logger::Here(), "Fatal exception: {}", e.what());
  MessageBoxW(nullptr, utils::utf8_to_wstring(e.what()).c_str(), L"Fatal Error", MB_OK | MB_ICONERROR);
  Logger::Shutdown();
  return -1;
}

void InitializeLogger() {
  LoggerConfig logger_config;
  logger_config.app_name_fallback = "dx12-game-with-framework";
  logger_config.file_path_mode = LoggerConfig::FilePathMode::WorkingDir;

  std::vector<std::unique_ptr<ILogSink>> sinks;
  sinks.push_back(std::make_unique<ConsoleSink>());
  sinks.push_back(std::make_unique<DebugSink>());

  try {
    sinks.push_back(std::make_unique<FileSink>(logger_config));
  } catch (const std::exception&) {
    // FileSink creation failed - continue with Console + Debug only
    OutputDebugStringA("[Logger] Warning: FileSink creation failed, continuing without file logging\n");
  }

  Logger::Init(std::move(logger_config), std::move(sinks));
}
