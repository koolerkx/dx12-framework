#pragma once
#include <cstdint>

struct SSAOConfig {
  bool enabled = true;
  float radius = 0.5f;      // Larger = wider/softer occlusion spread
  float bias = 0.025f;      // Larger = reduces false darkening on flat surfaces
  float intensity = 1.0f;   // Larger = darker occlusion shadows
  uint32_t sample_count = 32;  // 8-32, higher = less noise but slower
};
