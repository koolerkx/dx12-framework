/**
 * @file mesh_data_factory.h
 * @brief CPU-only mesh data generation (no GPU dependency).
 */
#pragma once

#include <array>
#include <cstdint>

#include "Framework/Math/Math.h"
#include "Framework/Render/mesh_data.h"

class MeshDataFactory {
 public:
  using CubeCornerColors = std::array<Math::Vector4, 8>;

  static MeshData CreateRectData();
  static MeshData CreateRoundedRectData(float corner_radius = 0.1f, uint32_t corner_segments = 8, float aspect_ratio = 1.0f);
  static MeshData CreateQuadData();
  static MeshData CreateCubeData();
  static MeshData CreateCubeData(const CubeCornerColors& corner_colors);
  static MeshData CreatePlaneData(uint32_t subdivisions_x = 1, uint32_t subdivisions_z = 1);
  static MeshData CreateSphereData(uint32_t segments = 32, uint32_t rings = 16);
  static MeshData CreateCylinderData(uint32_t segments = 8);
};
