#pragma once
#include <cstdint>

#include "Game/scene_defaults.h"
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
  OutlineConfig outline;
  VignetteConfig vignette;
  SceneDefaults scene_defaults;
};
