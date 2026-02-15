#pragma once
#include <d3d12.h>

#include <span>
#include <string_view>

#include "Frame/render_layer.h"
#include "Game/Component/render_settings.h"
#include "shader_types.h"
#include "vertex_types.h"

namespace Graphics {

struct ScanlineCubeShader {
  static constexpr ShaderId ID = 28;
  using VertexType = Vertex::Basic3DVertex;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "ScanlineCube";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/basic.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/scanline.ps.cso";
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
    float primary_r, primary_g, primary_b;
    float grid_divisions;
    float secondary_r, secondary_g, secondary_b;
    float grid_line_width;
    float scanline_speed;
    float scanline_width;
    float glow_intensity;
    float edge_glow_width;
  };
  static_assert(sizeof(Params) == 48);
};

}  // namespace Graphics
