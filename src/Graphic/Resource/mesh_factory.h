#pragma once
#include <d3d12.h>
#include <sys/stat.h>

#include "mesh.h"

class MeshFactory {
 public:
  // Create a unit cube (1x1x1) centered at origin
  static bool CreateCube(ID3D12Device* device, Mesh& out_mesh) {
    // Cube vertices with positions, UVs, and colors
    struct Vertex {
      float pos[3];
      float uv[2];
      float color[4];  // RGBA
    };

    // 24 vertices (4 per face for proper UVs and normals)
    Vertex vertices[] = {
      // Front face (Z+)
      {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},  // 0
      {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},   // 1
      {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},    // 2
      {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},   // 3

      // Back face (Z-)
      {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},   // 4
      {{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},  // 5
      {{-0.5f, 0.5f, -0.5f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},   // 6
      {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},    // 7

      // Top face (Y+)
      {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},   // 8
      {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},    // 9
      {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},   // 10
      {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},  // 11

      // Bottom face (Y-)
      {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},  // 12
      {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},   // 13
      {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},    // 14
      {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},   // 15

      // Right face (X+)
      {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},   // 16
      {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},  // 17
      {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},   // 18
      {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},    // 19

      // Left face (X-)
      {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},  // 20
      {{-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},   // 21
      {{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},    // 22
      {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},   // 23
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
      float color[4];  // RGBA
    };

    Vertex vertices[] = {
      {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},  // Bottom-left
      {{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},   // Bottom-right
      {{0.5f, 0.5f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},    // Top-right
      {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},   // Top-left
    };

    uint16_t indices[] = {0, 1, 2, 2, 3, 0};

    return out_mesh.Create(device, vertices, 4, indices, 6);
  }

  // Create a subdivided plane on XZ plane (Y=0)
  static bool CreatePlane(ID3D12Device* device, Mesh& out_mesh, uint32_t subdivisions_x = 1, uint32_t subdivisions_z = 1) {
    struct Vertex {
      float pos[3];
      float uv[2];
      float color[4];  // RGBA
    };

    // Calculate vertex count
    uint32_t vertex_count_x = subdivisions_x + 1;
    uint32_t vertex_count_z = subdivisions_z + 1;
    uint32_t total_vertices = vertex_count_x * vertex_count_z;

    std::vector<Vertex> vertices;
    vertices.reserve(total_vertices);

    // Generate vertices on XZ plane (Y=0)
    for (uint32_t z = 0; z < vertex_count_z; ++z) {
      for (uint32_t x = 0; x < vertex_count_x; ++x) {
        float u = static_cast<float>(x) / subdivisions_x;
        float v = static_cast<float>(z) / subdivisions_z;

        // Position: -0.5 to 0.5 in both X and Z
        float pos_x = u - 0.5f;
        float pos_z = v - 0.5f;

        vertices.push_back({{pos_x, 0.0f, pos_z}, {u, v}, {1.0f, 1.0f, 1.0f, 1.0f}});
      }
    }

    // Generate indices
    uint32_t index_count = subdivisions_x * subdivisions_z * 6;
    std::vector<uint16_t> indices;
    indices.reserve(index_count);

    for (uint32_t z = 0; z < subdivisions_z; ++z) {
      for (uint32_t x = 0; x < subdivisions_x; ++x) {
        uint16_t top_left = static_cast<uint16_t>(z * vertex_count_x + x);
        uint16_t top_right = static_cast<uint16_t>(top_left + 1);
        uint16_t bottom_left = static_cast<uint16_t>((z + 1) * vertex_count_x + x);
        uint16_t bottom_right = static_cast<uint16_t>(bottom_left + 1);

        // First triangle
        indices.push_back(top_left);
        indices.push_back(bottom_left);
        indices.push_back(top_right);

        // Second triangle
        indices.push_back(top_right);
        indices.push_back(bottom_left);
        indices.push_back(bottom_right);
      }
    }

    return out_mesh.Create(device, vertices.data(), vertices.size(), indices.data(), indices.size());
  }
};
