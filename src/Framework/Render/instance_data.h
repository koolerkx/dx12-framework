#pragma once
#include "Framework/Math/Math.h"

struct InstanceData {
  Math::Matrix4 world;
  Math::Vector4 color;
  Math::Vector2 uv_offset;
  Math::Vector2 uv_scale;
  Math::Vector4 overlay_color;
};
static_assert(sizeof(InstanceData) == 112);
