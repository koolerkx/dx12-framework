#pragma once

#include <cstdint>

struct MeshDescriptor {
  uint32_t vertex_offset;  // BaseVertexLocation (vertices, not bytes)
  uint32_t vertex_count;
  uint32_t index_offset;  // StartIndexLocation (indices, not bytes)
  uint32_t index_count;
};
static_assert(sizeof(MeshDescriptor) == 16);
