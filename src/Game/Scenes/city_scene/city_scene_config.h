#pragma once

#include <cstdint>

#include "Framework/Math/Math.h"

namespace CitySceneConfig {

[[maybe_unused]] constexpr float FBX_UNIT_SCALE = 0.01f;

[[maybe_unused]] constexpr struct {
  const char* skybox = "Content/skybox/sunflowers_puresky_standard_cubemap_4k.hdr";
  const char* map = "Content/map/City.yaml";
  const char* tower_selection_a = "Content/models/tower/selection-a.fbx";
  const char* tower_selection_b = "Content/models/tower/selection-b.fbx";
  const char* enemy_model = "Content/models/tower/enemy-ufo-a.fbx";
  const char* projectile_model = "Content/models/tower/weapon-ammo-bullet.fbx";
  const char* tower_model = "Content/models/tower2/tower-square-build-f.fbx";
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
  float move_speed = 1.0f;
  float size_scale = 0.75f;
  float agent_size_scale = 0.5f;
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

struct ArrivalExplosionConfig {
  const char* texture = "Content/textures/427_fire.png";
  DirectX::XMUINT2 sheet_size = {512, 576};
  DirectX::XMUINT2 frame_size = {64, 64};
  uint32_t start_row = 5;
  uint32_t frames_per_row = 8;
  uint32_t frame_count = 8;
  float fps = 12.0f;
  float sprite_size = 3.0f;
  float y_offset = 0.0f;
  Math::Vector3 emissive_color = {1.0f, 0.4f, 0.05f};
  float emissive_intensity = 3.0f;
};

struct BulletHitConfig {
  const char* texture = "Content/textures/427_fire.png";
  DirectX::XMUINT2 sheet_size = {512, 576};
  DirectX::XMUINT2 frame_size = {64, 64};
  uint32_t start_row = 5;
  uint32_t frames_per_row = 8;
  uint32_t frame_count = 8;
  float fps = 15.0f;
  float sprite_size = 0.8f;
  Math::Vector3 emissive_color = {1.0f, 0.8f, 0.3f};
  float emissive_intensity = 2.0f;
};

struct ExplosionSparksConfig {
  const char* texture = "Content/textures/sun_additive.png";
  uint32_t burst_count = 40;
  float lifetime = 0.4f;
  Math::Vector2 size = {0.15f, 0.15f};
  Math::Vector4 start_color = {2.0f, 2.0f, 2.0f, 1.0f};
  Math::Vector4 end_color = {1.0f, 0.4f, 0.05f, 0.0f};
  float start_speed = 15.0f;
  float speed_variation = 5.0f;
  Math::Vector3 gravity = {0.0f, -2.0f, 0.0f};
  float drag = 5.0f;
  float end_size = 0.0f;
  float lifetime_variation = 0.4f;
  float size_variation = 0.3f;
  float emissive_intensity = 4.0f;
  float spawn_radius = 0.2f;
};

struct BulletHitSparksConfig {
  const char* texture = "Content/textures/sun_additive.png";
  uint32_t burst_count = 15;
  float lifetime = 0.25f;
  Math::Vector2 size = {0.08f, 0.08f};
  Math::Vector4 start_color = {2.0f, 1.8f, 1.0f, 1.0f};
  Math::Vector4 end_color = {0.5f, 0.2f, 0.0f, 0.0f};
  float start_speed = 10.0f;
  float speed_variation = 4.0f;
  Math::Vector3 gravity = {0.0f, -3.0f, 0.0f};
  float drag = 6.0f;
  float end_size = 0.0f;
  float lifetime_variation = 0.3f;
  float size_variation = 0.25f;
  float emissive_intensity = 3.0f;
  float spawn_radius = 0.1f;
};

struct ArrivalScreenEffectConfig {
  float shake_intensity = 0.3f;
  float shake_duration = 0.4f;
  float chromatic_aberration_intensity = 1.0f;
};

}  // namespace CitySceneConfig
