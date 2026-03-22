/**
 * @file vertex_data.h
 * @brief CPU-only POD vertex structs for mesh data generation (no DX12 dependency).
 */
#pragma once

#include "Framework/Math/Math.h"

namespace VertexData {

struct ModelVertex {
  Math::Vector3 position;
  Math::Vector3 normal;
  Math::Vector2 texcoord;
  Math::Vector4 color;
  Math::Vector4 tangent;
};
static_assert(sizeof(ModelVertex) == 64);

struct SpriteVertex {
  Math::Vector3 position;
  Math::Vector2 tex_coord;
  Math::Vector4 color;
};
static_assert(sizeof(SpriteVertex) == 36);

}  // namespace VertexData
