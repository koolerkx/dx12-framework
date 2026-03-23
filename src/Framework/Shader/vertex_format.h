/**
 * @file vertex_format.h
 * @brief Vertex format and topology enums for shader composition without DX12 dependency.
 */
#pragma once

#include <cstdint>

enum class VertexFormat : uint8_t {
  Sprite,   // PositionTexCoordColor (36B)
  Basic3D,  // PositionNormalTexCoordColor (48B)
  Model,    // ModelVertex (64B)
  Line,     // PositionColor (28B)
};

enum class TopologyType : uint8_t {
  Triangle,
  Line,
  Point,
  Patch,
};
