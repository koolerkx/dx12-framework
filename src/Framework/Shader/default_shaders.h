/**
 * @file default_shaders.h
 * @brief Default shader definitions using VS+PS composition pattern.
 */
#pragma once

#include <string_view>
#include <tuple>

#include "Framework/Render/render_settings.h"
#include "shader_id.h"
#include "vertex_shaders.h"

namespace PS {

struct Sprite {
  static constexpr std::wstring_view PATH = L"Content/shaders/sprite.ps.cso";
};

struct Basic3D {
  static constexpr std::wstring_view PATH = L"Content/shaders/basic.ps.cso";
};

struct PBR {
  static constexpr std::wstring_view PATH = L"Content/shaders/pbr.ps.cso";
};

struct DebugLine {
  static constexpr std::wstring_view PATH = L"Content/shaders/debug_line.ps.cso";
};

}  // namespace PS

namespace Shaders {

struct Sprite {
  static constexpr std::string_view NAME = "Sprite";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexShader = VS::Sprite;
  using PixelShader = PS::Sprite;

  static Rendering::RenderSettings DefaultRenderSettings() {
    return {
      .blend_mode = Rendering::BlendMode::AlphaBlend,
      .sampler_type = Rendering::SamplerType::LinearWrap,
      .render_target_format = Rendering::RenderTargetFormat::HDR,
    };
  }
};

struct Basic3D {
  static constexpr std::string_view NAME = "Basic3D";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexShader = VS::Basic3D;
  using PixelShader = PS::Basic3D;

  static Rendering::RenderSettings DefaultRenderSettings() {
    return Rendering::RenderSettings::Opaque();
  }
};

struct PBR {
  static constexpr std::string_view NAME = "PBR";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexShader = VS::Model;
  using PixelShader = PS::PBR;

  static Rendering::RenderSettings DefaultRenderSettings() {
    return Rendering::RenderSettings::Opaque();
  }
};

struct DebugLine {
  static constexpr std::string_view NAME = "DebugLine";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexShader = VS::DebugLine;
  using PixelShader = PS::DebugLine;

  static Rendering::RenderSettings DefaultRenderSettings() {
    return {
      .blend_mode = Rendering::BlendMode::AlphaBlend,
      .sampler_type = Rendering::SamplerType::LinearWrap,
      .depth_test = true,
      .double_sided = true,
      .render_target_format = Rendering::RenderTargetFormat::HDR,
    };
  }
};

using DefaultShaders = std::tuple<Sprite, Basic3D, PBR, DebugLine>;

}  // namespace Shaders
