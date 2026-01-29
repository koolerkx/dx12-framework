#pragma once

struct HdrConfig {
  float exposure = 1.0f;
  bool enable_tone_mapping = true;
};

struct HdrDebug {
  bool collect_stats = false;
  bool stats_pending = false;
  bool debug_view = false;  // 0=normal, 1=pre-tone, 2=post-tone
};
