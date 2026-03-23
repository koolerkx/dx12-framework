/**
 * @file game_shaders.h
 * @brief Game content shader definitions using VS+PS composition pattern.
 */
#pragma once

#include <cstdint>
#include <string_view>

#include "Framework/Render/render_settings.h"
#include "Framework/Render/render_types.h"
#include "Framework/Shader/shader_id.h"
#include "Framework/Shader/vertex_shaders.h"

namespace PS {

struct NeonGrid {
  static constexpr std::wstring_view PATH = L"Content/shaders/neon_grid.ps.cso";

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
  static constexpr std::wstring_view PATH = L"Content/shaders/soft_particle.ps.cso";

  struct Params {
    uint32_t depth_srv_index;
    float emissive_intensity;
    float soft_distance;
    uint32_t _pad;
  };
  static_assert(sizeof(Params) == 16);
};

struct RadarRange {
  static constexpr std::wstring_view PATH = L"Content/shaders/radar_range.ps.cso";

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
  static constexpr std::wstring_view PATH = L"Content/shaders/laser_beam.ps.cso";

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
  static constexpr std::wstring_view PATH = L"Content/shaders/path_pulse.ps.cso";

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
  static constexpr std::wstring_view PATH = L"Content/shaders/ui_glass.ps.cso";
};

}  // namespace PS

namespace Shaders {

struct NeonGrid {
  static constexpr std::string_view NAME = "NeonGrid";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexShader = VS::Basic3D;
  using PixelShader = PS::NeonGrid;
  using Params = PixelShader::Params;
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
};

struct SoftParticle {
  static constexpr std::string_view NAME = "SoftParticle";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexShader = VS::Sprite;
  using PixelShader = PS::SoftParticle;
  using Params = PixelShader::Params;
};

struct RadarRange {
  static constexpr std::string_view NAME = "RadarRange";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexShader = VS::Basic3D;
  using PixelShader = PS::RadarRange;
  using Params = PixelShader::Params;
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
};

struct LaserBeam {
  static constexpr std::string_view NAME = "LaserBeam";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexShader = VS::Basic3D;
  using PixelShader = PS::LaserBeam;
  using Params = PixelShader::Params;
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
};

struct PathPulse {
  static constexpr std::string_view NAME = "PathPulse";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexShader = VS::Basic3D;
  using PixelShader = PS::PathPulse;
  using Params = PixelShader::Params;
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
};

struct UIGlass {
  static constexpr std::string_view NAME = "UIGlass";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexShader = VS::Sprite;
  using PixelShader = PS::UIGlass;

  static Rendering::RenderSettings DefaultRenderSettings() {
    return {
      .blend_mode = Rendering::BlendMode::AlphaBlend,
      .depth_test = false,
      .depth_write = false,
      .render_target_format = Rendering::RenderTargetFormat::SDR,
    };
  }
};

}  // namespace Shaders
