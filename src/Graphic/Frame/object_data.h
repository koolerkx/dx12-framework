/**
 * @file object_data.h
 * @brief Unified per-object data for GPU StructuredBuffer, replacing ObjectCB + InstanceData.
 */
#pragma once
#include <cstdint>

#include "Framework/Math/Math.h"

struct ObjectData {
  Math::Matrix4 world;
  Math::Vector4 color;
  Math::Vector2 uv_offset;
  Math::Vector2 uv_scale;
  Math::Vector4 overlay_color;
  uint32_t material_descriptor_index;
  uint32_t flags;
  uint32_t _pad[2];
};
static_assert(sizeof(ObjectData) == 128);
