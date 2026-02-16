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

}  // namespace Graphics
