#pragma once
#include <cstdint>

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
