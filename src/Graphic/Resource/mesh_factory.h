#pragma once
#include <d3d12.h>

#include <cmath>
#include <vector>

#include "mesh.h"

class MeshFactory {
 public:
  static bool CreateCube(ID3D12Device* device, Mesh& out_mesh) {
    struct Vertex {
      float pos[3];
      float normal[3];
      float uv[2];
      float color[4];
      float tangent[4];
    };

    // Tangent = U direction in world space, w = bitangent sign
    // bitangent = cross(normal, tangent.xyz) * tangent.w
    Vertex vertices[] = {
      // Front face (Z+): U=(1,0,0), cross(N,T)=(0,1,0), actual_B=(0,-1,0) → w=-1
      {{-0.5f, -0.5f, 0.5f}, {0, 0, 1}, {0.0f, 1.0f}, {1, 1, 1, 1}, {1, 0, 0, -1}},
      {{0.5f, -0.5f, 0.5f}, {0, 0, 1}, {1.0f, 1.0f}, {1, 1, 1, 1}, {1, 0, 0, -1}},
      {{0.5f, 0.5f, 0.5f}, {0, 0, 1}, {1.0f, 0.0f}, {1, 1, 1, 1}, {1, 0, 0, -1}},
      {{-0.5f, 0.5f, 0.5f}, {0, 0, 1}, {0.0f, 0.0f}, {1, 1, 1, 1}, {1, 0, 0, -1}},

      // Back face (Z-): U=(-1,0,0), cross(N,T)=(0,1,0), actual_B=(0,-1,0) → w=-1
      {{0.5f, -0.5f, -0.5f}, {0, 0, -1}, {0.0f, 1.0f}, {1, 1, 1, 1}, {-1, 0, 0, -1}},
      {{-0.5f, -0.5f, -0.5f}, {0, 0, -1}, {1.0f, 1.0f}, {1, 1, 1, 1}, {-1, 0, 0, -1}},
      {{-0.5f, 0.5f, -0.5f}, {0, 0, -1}, {1.0f, 0.0f}, {1, 1, 1, 1}, {-1, 0, 0, -1}},
      {{0.5f, 0.5f, -0.5f}, {0, 0, -1}, {0.0f, 0.0f}, {1, 1, 1, 1}, {-1, 0, 0, -1}},

      // Top face (Y+): U=(1,0,0), cross(N,T)=(0,0,-1), actual_B=(0,0,-1) → w=1
      {{-0.5f, 0.5f, 0.5f}, {0, 1, 0}, {0.0f, 1.0f}, {1, 1, 1, 1}, {1, 0, 0, 1}},
      {{0.5f, 0.5f, 0.5f}, {0, 1, 0}, {1.0f, 1.0f}, {1, 1, 1, 1}, {1, 0, 0, 1}},
      {{0.5f, 0.5f, -0.5f}, {0, 1, 0}, {1.0f, 0.0f}, {1, 1, 1, 1}, {1, 0, 0, 1}},
      {{-0.5f, 0.5f, -0.5f}, {0, 1, 0}, {0.0f, 0.0f}, {1, 1, 1, 1}, {1, 0, 0, 1}},

      // Bottom face (Y-): U=(1,0,0), cross(N,T)=(0,0,1), actual_B=(0,0,1) → w=1
      {{-0.5f, -0.5f, -0.5f}, {0, -1, 0}, {0.0f, 1.0f}, {1, 1, 1, 1}, {1, 0, 0, 1}},
      {{0.5f, -0.5f, -0.5f}, {0, -1, 0}, {1.0f, 1.0f}, {1, 1, 1, 1}, {1, 0, 0, 1}},
      {{0.5f, -0.5f, 0.5f}, {0, -1, 0}, {1.0f, 0.0f}, {1, 1, 1, 1}, {1, 0, 0, 1}},
      {{-0.5f, -0.5f, 0.5f}, {0, -1, 0}, {0.0f, 0.0f}, {1, 1, 1, 1}, {1, 0, 0, 1}},

      // Right face (X+): U=(0,0,-1), cross(N,T)=(0,1,0), actual_B=(0,-1,0) → w=-1
      {{0.5f, -0.5f, 0.5f}, {1, 0, 0}, {0.0f, 1.0f}, {1, 1, 1, 1}, {0, 0, -1, -1}},
      {{0.5f, -0.5f, -0.5f}, {1, 0, 0}, {1.0f, 1.0f}, {1, 1, 1, 1}, {0, 0, -1, -1}},
      {{0.5f, 0.5f, -0.5f}, {1, 0, 0}, {1.0f, 0.0f}, {1, 1, 1, 1}, {0, 0, -1, -1}},
      {{0.5f, 0.5f, 0.5f}, {1, 0, 0}, {0.0f, 0.0f}, {1, 1, 1, 1}, {0, 0, -1, -1}},

      // Left face (X-): U=(0,0,1), cross(N,T)=(0,1,0), actual_B=(0,-1,0) → w=-1
      {{-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0.0f, 1.0f}, {1, 1, 1, 1}, {0, 0, 1, -1}},
      {{-0.5f, -0.5f, 0.5f}, {-1, 0, 0}, {1.0f, 1.0f}, {1, 1, 1, 1}, {0, 0, 1, -1}},
      {{-0.5f, 0.5f, 0.5f}, {-1, 0, 0}, {1.0f, 0.0f}, {1, 1, 1, 1}, {0, 0, 1, -1}},
      {{-0.5f, 0.5f, -0.5f}, {-1, 0, 0}, {0.0f, 0.0f}, {1, 1, 1, 1}, {0, 0, 1, -1}},
    };

    uint16_t indices[] = {
      0,
      1,
      2,
      2,
      3,
      0,  // Front
      4,
      5,
      6,
      6,
      7,
      4,  // Back
      8,
      9,
      10,
      10,
      11,
      8,  // Top
      12,
      13,
      14,
      14,
      15,
      12,  // Bottom
      16,
      17,
      18,
      18,
      19,
      16,  // Right
      20,
      21,
      22,
      22,
      23,
      20,  // Left
    };

    return out_mesh.Create(device, vertices, 24, indices, 36);
  }

  static bool CreateQuad(ID3D12Device* device, Mesh& out_mesh) {
    struct Vertex {
      float pos[3];
      float normal[3];
      float uv[2];
      float color[4];
      float tangent[4];
    };

    Vertex vertices[] = {
      {{-0.5f, -0.5f, 0.0f}, {0, 0, -1}, {0.0f, 1.0f}, {1, 1, 1, 1}, {1, 0, 0, -1}},
      {{0.5f, -0.5f, 0.0f}, {0, 0, -1}, {1.0f, 1.0f}, {1, 1, 1, 1}, {1, 0, 0, -1}},
      {{0.5f, 0.5f, 0.0f}, {0, 0, -1}, {1.0f, 0.0f}, {1, 1, 1, 1}, {1, 0, 0, -1}},
      {{-0.5f, 0.5f, 0.0f}, {0, 0, -1}, {0.0f, 0.0f}, {1, 1, 1, 1}, {1, 0, 0, -1}},
    };

    uint16_t indices[] = {0, 1, 2, 2, 3, 0};

    return out_mesh.Create(device, vertices, 4, indices, 6);
  }

  static bool CreateRect(ID3D12Device* device, Mesh& out_mesh) {
    struct Vertex {
      float pos[3];
      float uv[2];
      float color[4];
    };

    Vertex vertices[] = {
      {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f}, {1, 1, 1, 1}},
      {{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f}, {1, 1, 1, 1}},
      {{0.5f, 0.5f, 0.0f}, {1.0f, 0.0f}, {1, 1, 1, 1}},
      {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f}, {1, 1, 1, 1}},
    };

    uint16_t indices[] = {0, 1, 2, 2, 3, 0};

    return out_mesh.Create(device, vertices, 4, indices, 6);
  }

  static bool CreatePlane(ID3D12Device* device, Mesh& out_mesh, uint32_t subdivisions_x = 1, uint32_t subdivisions_z = 1) {
    struct Vertex {
      float pos[3];
      float normal[3];
      float uv[2];
      float color[4];
      float tangent[4];
    };

    uint32_t vertex_count_x = subdivisions_x + 1;
    uint32_t vertex_count_z = subdivisions_z + 1;

    std::vector<Vertex> vertices;
    vertices.reserve(vertex_count_x * vertex_count_z);

    // Plane on XZ, Y=0, normal=(0,1,0), tangent along U=(1,0,0)
    // cross(N,T)=(0,0,-1), V goes +Z so actual_B=(0,0,1) → w=-1
    for (uint32_t z = 0; z < vertex_count_z; ++z) {
      for (uint32_t x = 0; x < vertex_count_x; ++x) {
        float u = static_cast<float>(x) / subdivisions_x;
        float v = static_cast<float>(z) / subdivisions_z;
        float pos_x = u - 0.5f;
        float pos_z = v - 0.5f;

        vertices.push_back({{pos_x, 0.0f, pos_z}, {0, 1, 0}, {u, v}, {1, 1, 1, 1}, {1, 0, 0, -1}});
      }
    }

    uint32_t index_count = subdivisions_x * subdivisions_z * 6;
    std::vector<uint16_t> indices;
    indices.reserve(index_count);

    for (uint32_t z = 0; z < subdivisions_z; ++z) {
      for (uint32_t x = 0; x < subdivisions_x; ++x) {
        uint16_t top_left = static_cast<uint16_t>(z * vertex_count_x + x);
        uint16_t top_right = static_cast<uint16_t>(top_left + 1);
        uint16_t bottom_left = static_cast<uint16_t>((z + 1) * vertex_count_x + x);
        uint16_t bottom_right = static_cast<uint16_t>(bottom_left + 1);

        indices.push_back(top_left);
        indices.push_back(bottom_left);
        indices.push_back(top_right);

        indices.push_back(top_right);
        indices.push_back(bottom_left);
        indices.push_back(bottom_right);
      }
    }

    return out_mesh.Create(device, vertices.data(), vertices.size(), indices.data(), indices.size());
  }

  static bool CreateSphere(ID3D12Device* device, Mesh& out_mesh, uint32_t segments = 32, uint32_t rings = 16) {
    struct Vertex {
      float pos[3];
      float normal[3];
      float uv[2];
      float color[4];
      float tangent[4];
    };

    constexpr float PI = 3.14159265358979323846f;
    constexpr float RADIUS = 0.5f;

    std::vector<Vertex> vertices;
    vertices.reserve((rings + 1) * (segments + 1));

    for (uint32_t ring = 0; ring <= rings; ++ring) {
      float v = static_cast<float>(ring) / rings;
      float theta = v * PI;
      float sin_theta = sinf(theta);
      float cos_theta = cosf(theta);

      for (uint32_t seg = 0; seg <= segments; ++seg) {
        float u = static_cast<float>(seg) / segments;
        float phi = u * 2.0f * PI;
        float sin_phi = sinf(phi);
        float cos_phi = cosf(phi);

        float nx = sin_theta * cos_phi;
        float ny = cos_theta;
        float nz = sin_theta * sin_phi;

        // Tangent = dP/dU direction (along longitude)
        float tx = -sin_phi;
        float ty = 0.0f;
        float tz = cos_phi;
        float len = sqrtf(tx * tx + ty * ty + tz * tz);
        if (len > 0.0f) {
          tx /= len;
          ty /= len;
          tz /= len;
        }

        vertices.push_back({
          {nx * RADIUS, ny * RADIUS, nz * RADIUS},
          {nx, ny, nz},
          {u, v},
          {1.0f, 1.0f, 1.0f, 1.0f},
          {tx, ty, tz, 1.0f},
        });
      }
    }

    std::vector<uint16_t> indices;
    indices.reserve(rings * segments * 6);

    for (uint32_t ring = 0; ring < rings; ++ring) {
      for (uint32_t seg = 0; seg < segments; ++seg) {
        uint16_t current = static_cast<uint16_t>(ring * (segments + 1) + seg);
        uint16_t next = static_cast<uint16_t>(current + 1);
        uint16_t below = static_cast<uint16_t>(current + segments + 1);
        uint16_t below_next = static_cast<uint16_t>(below + 1);

        indices.push_back(current);
        indices.push_back(next);
        indices.push_back(below);

        indices.push_back(next);
        indices.push_back(below_next);
        indices.push_back(below);
      }
    }

    return out_mesh.Create(device, vertices.data(), vertices.size(), indices.data(), indices.size());
  }
};
