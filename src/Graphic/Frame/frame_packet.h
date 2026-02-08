#pragma once
#include <cstdint>
#include <vector>

#include "Framework/Core/color.h"
#include "camera_data.h"
#include "draw_command.h"

enum class BackgroundMode : uint8_t { ClearColor, Skybox };

struct BackgroundConfig {
  BackgroundMode mode = BackgroundMode::ClearColor;
  Color clear_color = colors::DarkSlateGray;
  uint32_t cubemap_srv_index = UINT32_MAX;
};

struct FramePacket {
  CameraData main_camera;
  CameraData ui_camera;
  BackgroundConfig background;
  std::vector<DrawCommand> commands;

  void Clear() {
    commands.clear();
  }

  void AddCommand(DrawCommand cmd) {
    commands.emplace_back(std::move(cmd));
  }
};
