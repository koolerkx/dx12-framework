#pragma once
#include <d3d12.h>

#include <span>
#include <string_view>

#include "Frame/render_layer.h"
#include "Game/Component/render_settings.h"
#include "shader_types.h"
#include "vertex_types.h"

namespace Graphics {

struct NeonGridShader {
  static constexpr ShaderId ID = 28;
  using VertexType = Vertex::Basic3DVertex;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "NeonGrid";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/basic.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/neon_grid.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }

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

struct SoftParticleShader {
  static constexpr ShaderId ID = 29;
  using VertexType = Vertex::SpriteInstanced;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "SoftParticle";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/sprite_instanced.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/soft_particle.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }

  struct Params {
    uint32_t depth_srv_index;
    float emissive_intensity;
    float soft_distance;
    uint32_t _pad;
  };
  static_assert(sizeof(Params) == 16);
};

struct RadarRangeShader {
  static constexpr ShaderId ID = 30;
  using VertexType = Vertex::Basic3DVertex;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "RadarRange";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/basic.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/radar_range.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }

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

struct LaserBeamShader {
  static constexpr ShaderId ID = 31;
  using VertexType = Vertex::Basic3DVertex;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "LaserBeam";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/basic.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/laser_beam.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }

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

struct PathPulseShader {
  static constexpr ShaderId ID = 32;
  using VertexType = Vertex::Basic3DVertex;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "PathPulse";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/basic.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/path_pulse.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }

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

struct UIGlassShader {
  static constexpr ShaderId ID = 34;
  using VertexType = Vertex::SpriteVertex;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "UIGlass";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/sprite.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/ui_glass.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static Rendering::RenderSettings DefaultRenderSettings() {
    return {
      .blend_mode = Rendering::BlendMode::AlphaBlend,
      .depth_test = false,
      .depth_write = false,
      .render_target_format = Rendering::RenderTargetFormat::SDR,
    };
  }

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

}  // namespace Graphics
