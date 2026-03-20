#pragma once

#include <cstdint>

#include "Framework/Math/Math.h"

using Math::Vector3;

enum class MaterialFlags : uint32_t {
  None = 0,
  AlphaTest = 1u << 0,
  DoubleSided = 1u << 1,
  RimShadowAffected = 1u << 2,
  HasNormalMap = 1u << 3,
  HasMetallicRoughnessMap = 1u << 4,
  HasEmissiveMap = 1u << 5,
};

namespace flags {
template <typename E>
constexpr uint32_t Combine(E flag) {
  return static_cast<uint32_t>(flag);
}
template <typename E, typename... Es>
constexpr uint32_t Combine(E flag, Es... rest) {
  return static_cast<uint32_t>(flag) | Combine(rest...);
}
template <typename E>
constexpr uint32_t If(bool condition, E flag) {
  return condition ? static_cast<uint32_t>(flag) : 0u;
}
}  // namespace flags

struct MaterialDescriptor {
  uint32_t albedo_texture_index;
  uint32_t normal_texture_index;
  uint32_t metallic_roughness_index;
  uint32_t emissive_texture_index;

  uint32_t flags;
  float specular_intensity;
  float specular_power;
  float rim_intensity;

  float rim_power;
  Vector3 rim_color;

  float metallic_factor;
  float roughness_factor;
  uint32_t sampler_index;
  float _pad0;

  Vector3 emissive_factor;
  float _pad1;
};
static_assert(sizeof(MaterialDescriptor) == 80);
