#pragma once
#include <cstdint>

#include "Graphic/Frame/frame_packet.h"

struct SceneDefaults {
  float light_azimuth = 45.0f;
  float light_elevation = 45.0f;
  float light_intensity = 0.8f;
  float light_color[3] = {1.0f, 1.0f, 1.0f};
  float ambient_intensity = 0.2f;
  float ambient_color[3] = {1.0f, 1.0f, 1.0f};

  bool shadow_enabled = true;
  uint32_t shadow_resolution = 2048;
  uint32_t shadow_cascade_count = 3;
  ShadowAlgorithm shadow_algorithm = ShadowAlgorithm::PCF3x3;
  float shadow_distance = 100.0f;
  float light_distance = 100.0f;
  float cascade_blend_range = 0.1f;
  float shadow_color[3] = {0.0f, 0.0f, 0.0f};
  float light_size = 1.0f;

  BackgroundMode background_mode = BackgroundMode::ClearColor;
  float clear_color[4] = {0.184f, 0.310f, 0.310f, 1.0f};
};
