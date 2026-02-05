#pragma once
#include <cstdint>

// Mutually exclusive rendering layer
enum class RenderLayer : uint8_t {
  Opaque = 0,
  Transparent = 1,
  UI = 2,
  Debug = 3,
  COUNT
};

// Combinable render tags (bitmask)
enum class RenderTag : uint32_t {
  None = 0,
  CastShadow = 1 << 0,
  ReceiveShadow = 1 << 1,
  Outline = 1 << 2,
  Glow = 1 << 3,
};

using RenderTagMask = uint32_t;

// Bitmask operations
inline constexpr RenderTag operator|(RenderTag a, RenderTag b) {
  return static_cast<RenderTag>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline constexpr bool HasTag(RenderTagMask mask, RenderTag tag) {
  return (mask & static_cast<uint32_t>(tag)) != 0;
}

inline constexpr bool HasAllTags(RenderTagMask mask, RenderTagMask required) {
  return (mask & required) == required;
}
