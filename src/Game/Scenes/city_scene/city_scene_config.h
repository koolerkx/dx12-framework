#pragma once

#include "Framework/Math/Math.h"

namespace CitySceneConfig {

[[maybe_unused]] constexpr float FBX_UNIT_SCALE = 0.01f;

[[maybe_unused]] constexpr struct {
  const char* skybox = "Content/skybox/sunflowers_puresky_standard_cubemap_4k.hdr";
  const char* map = "Content/map/City.yaml";
  const char* tower_selection_a = "Content/models/tower/selection-a.fbx";
  const char* tower_selection_b = "Content/models/tower/selection-b.fbx";
} PATHS;

struct LightConfig {
  float azimuth = 45.0f;
  float elevation = 55.0f;
};

struct CameraConfig {
  Math::Vector3 position = {5.0f, 20.0f, -20.0f};
  Math::Vector3 rotation_degrees = {45.0f, 0.0f, 0.0f};
  float movement_speed = 20.0f;
  float rotation_speed = 1.5f;
  float smoothness = 8.0f;
};

struct NavGridConfig {
  float cell_size = 0.25f;
  float block_threshold = 0.5f;
  bool show_debug_grid = true;
};

struct DebugDrawConfig {
  float grid_size = 30.0f;
  float grid_cell_size = 1.0f;
  float grid_y_level = 0.0f;
  float axis_length = 2.0f;
  float nav_grid_y_level = 0.1f;
};

struct EnemyConfig {
  Math::Vector3 spawn_position = {0.0f, 0.5f, 10.0f};
  float scale = 0.5f;
  float move_speed = 3.0f;
  Math::Vector2 initial_target_xz = {5.0f, 0.0f};
};

struct BorderWallConfig {
  float cube_y = 0.4f;
  float margin = 1.0f;
};

struct SpawnCubeConfig {
  float y_offset = 1.0f;
  float scale = 0.5f;
};

struct HpBarConfig {
  float bar_width = 1.0f;
  float y_offset = 1.2f;
  float fill_px_w = 271.0f;
  float fill_px_h = 21.0f;
  const char* texture_back = "Content/textures/hp_bar_back.png";
  const char* texture_main = "Content/textures/hp_bar_main.png";
};

}  // namespace CitySceneConfig
