/**
 * @file post_process_config.h
 * @brief All post-processing effect configurations.
 */
#pragma once

#include <cstdint>

struct SSAOConfig {
  bool enabled = true;
  float radius = 0.5f;      // Larger = wider/softer occlusion spread
  float bias = 0.025f;      // Larger = reduces false darkening on flat surfaces
  float intensity = 1.0f;   // Larger = darker occlusion shadows
  uint32_t sample_count = 32;  // 8-32, higher = less noise but slower
};

struct FogConfig {
  bool enabled = false;
  float density = 0.02f;
  float height_falloff = 0.5f;
  float base_height = 0.0f;
  float max_distance = 500.0f;
  float fog_color[3] = {0.7f, 0.75f, 0.8f};
};

struct VignetteConfig {
  bool enabled = false;
  float intensity = 0.5f;
  float radius = 0.5f;
  float softness = 0.5f;
  float roundness = 1.0f;
  float vignette_color[3] = {0.0f, 0.0f, 0.0f};
};

struct OutlineConfig {
  bool enabled = false;
  float depth_weight = 1.0f;
  float normal_weight = 1.0f;
  float edge_threshold = 0.1f;
  float depth_falloff = 0.01f;
  float thickness = 1.0f;
  float outline_color[3] = {0.0f, 0.0f, 0.0f};
};

// Transient runtime effect — do NOT add to GraphicInitProps or config files.
// Mutate only via Graphic::GetChromaticAberrationConfig() from game layer triggers.
struct ChromaticAberrationConfig {
  float intensity = 0.0f;
};

struct SMAAConfig {
  bool enabled = true;
};

struct HdrDebug {
  bool collect_stats = false;
  bool stats_pending = false;
  bool debug_view = false;  // 0=normal, 1=pre-tone, 2=post-tone
};

struct DepthViewConfig {
  bool enabled = false;
  float near_plane = 0.1f;
  float far_plane = 1000.0f;
};

struct BloomConfig {
  bool enabled = true;
  uint32_t mip_levels = 5;
  float threshold = 1.0f;
  float intensity = 0.5f;
};
