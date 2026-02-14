#pragma once

struct VignetteConfig {
  bool enabled = false;
  float intensity = 0.5f;
  float radius = 0.5f;
  float softness = 0.5f;
  float roundness = 1.0f;
  float vignette_color[3] = {0.0f, 0.0f, 0.0f};
};
