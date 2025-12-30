#pragma once
#include <d3d12.h>

#include "mesh.h"

class MeshFactory {
 public:
  // Create a unit cube (1x1x1) centered at origin
  static bool CreateCube(ID3D12Device* device, Mesh& out_mesh) {
    // Cube vertices with positions and UVs
    struct Vertex {
      float pos[3];
      float uv[2];
    };

    // 24 vertices (4 per face for proper UVs and normals)
    Vertex vertices[] = {
      // Front face (Z+)
      {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}},  // 0
      {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f}},   // 1
      {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},    // 2
      {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}},   // 3

      // Back face (Z-)
      {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},   // 4
      {{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},  // 5
      {{-0.5f, 0.5f, -0.5f}, {1.0f, 0.0f}},   // 6
      {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f}},    // 7

      // Top face (Y+)
      {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}},   // 8
      {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},    // 9
      {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f}},   // 10
      {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f}},  // 11

      // Bottom face (Y-)
      {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},  // 12
      {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},   // 13
      {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}},    // 14
      {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}},   // 15

      // Right face (X+)
      {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}},   // 16
      {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},  // 17
      {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f}},   // 18
      {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}},    // 19

      // Left face (X-)
      {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},  // 20
      {{-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f}},   // 21
      {{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},    // 22
      {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f}},   // 23
    };

    // 36 indices (6 faces * 2 triangles * 3 vertices)
    uint16_t indices[] = {// Front
      0,
      1,
      2,
      2,
      3,
      0,
      // Back
      4,
      5,
      6,
      6,
      7,
      4,
      // Top
      8,
      9,
      10,
      10,
      11,
      8,
      // Bottom
      12,
      13,
      14,
      14,
      15,
      12,
      // Right
      16,
      17,
      18,
      18,
      19,
      16,
      // Left
      20,
      21,
      22,
      22,
      23,
      20};

    return out_mesh.Create(device, vertices, 24, indices, 36);
  }

  // Create a unit quad for testing
  static bool CreateQuad(ID3D12Device* device, Mesh& out_mesh) {
    struct Vertex {
      float pos[3];
      float uv[2];
    };

    Vertex vertices[] = {
      {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f}},  // Bottom-left
      {{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f}},   // Bottom-right
      {{0.5f, 0.5f, 0.0f}, {1.0f, 0.0f}},    // Top-right
      {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f}},   // Top-left
    };

    uint16_t indices[] = {0, 1, 2, 2, 3, 0};

    return out_mesh.Create(device, vertices, 4, indices, 6);
  }
};
