#pragma once

#include "Framework/Math/Math.h"

struct GPUInstanceData {
  Math::Matrix4 world;
  Math::Matrix4 normal_matrix;
  Math::Vector4 color;
};
static_assert(sizeof(GPUInstanceData) == 144);
