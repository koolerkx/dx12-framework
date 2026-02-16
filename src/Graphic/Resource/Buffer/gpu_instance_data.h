#pragma once

#include "Framework/Math/Math.h"

struct GPUInstanceData {
  Math::Matrix4 world;
  Math::Matrix4 normal_matrix;
  Math::Vector4 color;
  Math::Vector4 overlay_color;
};
static_assert(sizeof(GPUInstanceData) == 160);
