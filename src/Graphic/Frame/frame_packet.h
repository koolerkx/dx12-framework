#pragma once
#include <vector>

#include "camera_data.h"
#include "draw_command.h"

struct FramePacket {
  CameraData main_camera;
  std::vector<DrawCommand> commands;

  void Clear() {
    commands.clear();
  }

  void AddCommand(DrawCommand cmd) {
    commands.emplace_back(std::move(cmd));
  }
};
