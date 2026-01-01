#pragma once

#include <cassert>
#include <cstdint>

namespace Rendering {

enum class BlendMode : uint8_t { Opaque, AlphaBlend, Additive, Premultiplied };

enum class SamplerType : uint8_t { PointWrap = 0, LinearWrap = 1, AnisotropicWrap = 2, PointClamp = 3, LinearClamp = 4 };

struct RenderSettings {
  BlendMode blend_mode = BlendMode::AlphaBlend;
  SamplerType sampler_type = SamplerType::LinearWrap;
  bool depth_test = false;
  bool depth_write = false;
  bool double_sided = false;

  void Validate() const {
    // depth_write requires depth_test to be enabled
    assert(!(depth_write && !depth_test) && "depth_write requires depth_test to be enabled");

    // sampler_type must be in valid range [0, 4]
    assert(static_cast<uint8_t>(sampler_type) <= 4 && "sampler_type out of range");
  }

  uint32_t GetCacheKey() const {
    // Generate bitfield-based cache key
    // Layout: [blend_mode:2][sampler_type:3][depth_test:1][depth_write:1][double_sided:1]
    uint32_t key = 0;

    key |= (static_cast<uint32_t>(blend_mode) & 0x3);         // bits 0-1
    key |= (static_cast<uint32_t>(sampler_type) & 0x7) << 2;  // bits 2-4
    key |= (depth_test ? 1u : 0u) << 5;                       // bit 5
    key |= (depth_write ? 1u : 0u) << 6;                      // bit 6
    key |= (double_sided ? 1u : 0u) << 7;                     // bit 7

    return key;
  }

  // Helper methods for common presets
  static RenderSettings Opaque() {
    RenderSettings settings;
    settings.blend_mode = BlendMode::Opaque;
    settings.depth_test = true;
    settings.depth_write = true;
    return settings;
  }

  static RenderSettings Transparent() {
    RenderSettings settings;
    settings.blend_mode = BlendMode::AlphaBlend;
    settings.depth_test = true;
    settings.depth_write = false;
    return settings;
  }

  static RenderSettings UI() {
    RenderSettings settings;
    settings.blend_mode = BlendMode::AlphaBlend;
    settings.depth_test = false;
    settings.depth_write = false;
    return settings;
  }
};

}  // namespace Rendering
