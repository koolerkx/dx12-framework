#pragma once

struct FogConfig {
  bool enabled = false;
  float density = 0.02f;
  float height_falloff = 0.5f;
  float base_height = 0.0f;
  float max_distance = 500.0f;
  float fog_color[3] = {0.7f, 0.75f, 0.8f};
};
