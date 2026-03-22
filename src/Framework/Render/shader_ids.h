/**
 * @file shader_ids.h
 * @brief Shader ID constants and lightweight shader descriptors for Game layer use.
 *
 * Game layer uses these instead of including Graphic/Pipeline/shader_descriptors.h.
 * Graphic layer static_asserts that IDs match the full shader definitions.
 */
#pragma once

#include <cstdint>

#include "Framework/Render/render_settings.h"
#include "Framework/Render/render_types.h"
#include "Framework/Render/shader_types.h"

namespace Shaders {

// Shaders used by Game with SetShaderWithParams<T>() template.
// These carry ID + Params + optional RENDER_LAYER/RENDER_TAGS/DefaultRenderSettings.

struct NeonGrid {
  static constexpr Graphics::ShaderId ID = 28;
  static constexpr RenderLayer RENDER_LAYER = RenderLayer::Transparent;
  static constexpr RenderTagMask RENDER_TAGS = 0;

  static Rendering::RenderSettings DefaultRenderSettings() {
    return {
      .blend_mode = Rendering::BlendMode::Additive,
      .depth_test = true,
      .double_sided = true,
      .render_target_format = Rendering::RenderTargetFormat::HDR,
    };
  }

  struct Params {
    float grid_r, grid_g, grid_b;
    float grid_divisions;
    float fill_r, fill_g, fill_b;
    float fill_opacity;
    float grid_line_width;
    float glow_intensity;
  };
  static_assert(sizeof(Params) == 40);
};

struct SoftParticle {
  static constexpr Graphics::ShaderId ID = 29;

  struct Params {
    uint32_t depth_srv_index;
    float emissive_intensity;
    float soft_distance;
    uint32_t _pad;
  };
  static_assert(sizeof(Params) == 16);
};

struct RadarRange {
  static constexpr Graphics::ShaderId ID = 30;
  static constexpr RenderLayer RENDER_LAYER = RenderLayer::Transparent;
  static constexpr RenderTagMask RENDER_TAGS = 0;

  static Rendering::RenderSettings DefaultRenderSettings() {
    return {
      .blend_mode = Rendering::BlendMode::Additive,
      .depth_test = true,
      .double_sided = true,
      .render_target_format = Rendering::RenderTargetFormat::HDR,
    };
  }

  struct Params {
    float radar_r, radar_g, radar_b;
    float scan_speed;
    float ring_count;
    float opacity;
    float emissive_intensity;
    float ring_width;
  };
  static_assert(sizeof(Params) == 32);
};

struct LaserBeam {
  static constexpr Graphics::ShaderId ID = 31;
  static constexpr RenderLayer RENDER_LAYER = RenderLayer::Transparent;
  static constexpr RenderTagMask RENDER_TAGS = 0;

  static Rendering::RenderSettings DefaultRenderSettings() {
    return {
      .blend_mode = Rendering::BlendMode::Additive,
      .depth_test = true,
      .double_sided = true,
      .render_target_format = Rendering::RenderTargetFormat::HDR,
    };
  }

  struct Params {
    float laser_r, laser_g, laser_b;
    float emissive_intensity;
    float pulse_speed;
    float pulse_frequency;
    float beam_width;
    float end_fade_ratio;
    float end_fade_power;
    float _pad[3] = {0, 0, 0};
  };
  static_assert(sizeof(Params) == 48);
};

struct PathPulse {
  static constexpr Graphics::ShaderId ID = 32;
  static constexpr RenderLayer RENDER_LAYER = RenderLayer::Transparent;
  static constexpr RenderTagMask RENDER_TAGS = 0;

  static Rendering::RenderSettings DefaultRenderSettings() {
    return {
      .blend_mode = Rendering::BlendMode::Additive,
      .depth_test = true,
      .double_sided = true,
      .render_target_format = Rendering::RenderTargetFormat::HDR,
    };
  }

  struct Params {
    float pulse_r, pulse_g, pulse_b;
    float emissive_intensity = 8.0f;
    float pulse_speed = 4.0f;
    float pulse_frequency = 10.0f;
    float pulse_width = 1.0f;
    float distance_offset = 0;
    float segment_length = 0;
    float _pad[3] = {0, 0, 0};
  };
  static_assert(sizeof(Params) == 48);
};

struct UIGlass {
  static constexpr Graphics::ShaderId ID = 34;

  static Rendering::RenderSettings DefaultRenderSettings() {
    return {
      .blend_mode = Rendering::BlendMode::AlphaBlend,
      .depth_test = false,
      .depth_write = false,
      .render_target_format = Rendering::RenderTargetFormat::SDR,
    };
  }
};

// Plain ID constants for shaders used without SetShaderWithParams<T>().

namespace Id {

constexpr Graphics::ShaderId SPRITE = 0;
constexpr Graphics::ShaderId BASIC_3D = 4;
constexpr Graphics::ShaderId DEBUG_LINE = 5;
constexpr Graphics::ShaderId PBR = 11;
constexpr Graphics::ShaderId SOFT_PARTICLE = SoftParticle::ID;
constexpr Graphics::ShaderId UI_GLASS = UIGlass::ID;

}  // namespace Id

}  // namespace Shaders
