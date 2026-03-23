#pragma once
#include <cstdint>
#include <string>

#include "Game/scene_defaults.h"
#include "Graphic/Rendering/fog_config.h"
#include "Graphic/Rendering/hdr_config.h"
#include "Graphic/Rendering/outline_config.h"
#include "Graphic/Rendering/smaa_config.h"
#include "Graphic/Rendering/ssao_config.h"
#include "Graphic/Rendering/vignette_config.h"

struct AppConfig {
  uint32_t window_width = 1920;
  uint32_t window_height = 1080;
  bool vsync = false;
  BloomConfig bloom;
  SSAOConfig ssao;
  SMAAConfig smaa;
  FogConfig fog;
  OutlineConfig outline;
  VignetteConfig vignette;
  SceneDefaults scene_defaults;
  std::string startup_scene = "title";
  bool debug_draw_enabled = false;
  float editor_bg_color[4] = {0.15f, 0.15f, 0.15f, 1.0f};
};
