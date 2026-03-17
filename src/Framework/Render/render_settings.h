#pragma once

#include <cassert>
#include <cstdint>

namespace Rendering {

enum class BlendMode : uint8_t { Opaque, AlphaBlend, Additive, Premultiplied };

enum class SamplerType : uint8_t { PointWrap = 0, LinearWrap = 1, AnisotropicWrap = 2, PointClamp = 3, LinearClamp = 4 };

enum class RenderTargetFormat : uint8_t {
  SDR = 0,  // R8G8B8A8_UNORM (swapchain)
  HDR = 1   // R16G16B16A16_FLOAT (HDR render target)
};

struct RenderSettings {
  BlendMode blend_mode = BlendMode::AlphaBlend;
  SamplerType sampler_type = SamplerType::LinearWrap;
  bool depth_test = false;
  bool depth_write = false;
  bool double_sided = false;
  bool wireframe = false;
  RenderTargetFormat render_target_format = RenderTargetFormat::HDR;

  void Validate() const {
    assert(!(depth_write && !depth_test) && "depth_write requires depth_test to be enabled");
    assert(static_cast<uint8_t>(sampler_type) <= 4 && "sampler_type out of range");
  }

  uint32_t GetCacheKey() const {
    uint32_t key = 0;
    key |= (static_cast<uint32_t>(blend_mode) & 0x3);
    key |= (static_cast<uint32_t>(sampler_type) & 0x7) << 2;
    key |= (depth_test ? 1u : 0u) << 5;
    key |= (depth_write ? 1u : 0u) << 6;
    key |= (double_sided ? 1u : 0u) << 7;
    key |= (static_cast<uint32_t>(render_target_format) & 0x1) << 8;
    key |= (wireframe ? 1u : 0u) << 9;
    return key;
  }

  static RenderSettings Opaque() {
    RenderSettings settings;
    settings.blend_mode = BlendMode::Opaque;
    settings.depth_test = true;
    settings.depth_write = true;
    settings.render_target_format = RenderTargetFormat::HDR;
    return settings;
  }

  static RenderSettings Transparent() {
    RenderSettings settings;
    settings.blend_mode = BlendMode::AlphaBlend;
    settings.depth_test = true;
    settings.depth_write = false;
    settings.render_target_format = RenderTargetFormat::HDR;
    return settings;
  }

  static RenderSettings UI() {
    RenderSettings settings;
    settings.blend_mode = BlendMode::AlphaBlend;
    settings.depth_test = false;
    settings.depth_write = false;
    settings.render_target_format = RenderTargetFormat::SDR;
    return settings;
  }
};

}  // namespace Rendering
