#pragma once

struct OutlineConfig {
  bool enabled = false;
  float depth_weight = 1.0f;
  float normal_weight = 1.0f;
  float edge_threshold = 0.1f;
  float depth_falloff = 0.01f;
  float thickness = 1.0f;
  float outline_color[3] = {0.0f, 0.0f, 0.0f};
};
