#pragma once
#include <cstdint>
#include <vector>

#include "Framework/Core/color.h"
#include "Framework/Math/Math.h"
#include "Graphic/Render/shadow_config.h"
#include "camera_data.h"
#include "draw_command.h"


enum class BackgroundMode : uint8_t { ClearColor, Skybox };

struct BackgroundConfig {
  BackgroundMode mode = BackgroundMode::ClearColor;
  Color clear_color = colors::DarkSlateGray;
  uint32_t cubemap_srv_index = UINT32_MAX;
};

struct LightingConfig {
  Math::Vector3 direction = Math::Vector3(0.0f, -1.0f, 0.0f);
  float intensity = 1.0f;
  Math::Vector3 directional_color = Math::Vector3(1.0f, 1.0f, 1.0f);
  float ambient_intensity = 0.2f;
  Math::Vector3 ambient_color = Math::Vector3(1.0f, 1.0f, 1.0f);
};

enum class ShadowAlgorithm : uint32_t {
  Hard = 0,
  PCF3x3 = 1,
  PoissonDisk = 2,
  RotatedPoissonDisk = 3,
};

struct ShadowConfig {
  uint32_t resolution = 2048;
  uint32_t cascade_count = ShadowCascadeConfig::DEFAULT_CASCADE_COUNT;
  float cascade_depth_bias[ShadowCascadeConfig::MAX_CASCADES] = {ShadowReceiverDefaults::DEPTH_BIAS,
    ShadowReceiverDefaults::DEPTH_BIAS,
    ShadowReceiverDefaults::DEPTH_BIAS,
    ShadowReceiverDefaults::DEPTH_BIAS};
  float cascade_normal_bias[ShadowCascadeConfig::MAX_CASCADES] = {ShadowReceiverDefaults::NORMAL_BIAS,
    ShadowReceiverDefaults::NORMAL_BIAS,
    ShadowReceiverDefaults::NORMAL_BIAS,
    ShadowReceiverDefaults::NORMAL_BIAS};
  float shadow_distance = 100.0f;
  float light_distance = 100.0f;
  float cascade_blend_range = 0.1f;
  ShadowAlgorithm algorithm = ShadowAlgorithm::PCF3x3;
  Math::Vector3 shadow_color = Math::Vector3(0.0f, 0.0f, 0.0f);
  bool enabled = true;
};

struct FramePacket {
  CameraData main_camera;
  CameraData ui_camera;
  BackgroundConfig background;
  LightingConfig lighting;
  ShadowConfig shadow;
  std::vector<DrawCommand> commands;

  void Clear() {
    commands.clear();
  }

  void AddCommand(DrawCommand cmd) {
    commands.emplace_back(std::move(cmd));
  }
};
