#pragma once
#include <d3d12.h>

#include <cstdint>

struct MeshGeometry {
  D3D12_VERTEX_BUFFER_VIEW vbv{};
  D3D12_INDEX_BUFFER_VIEW ibv{};
  uint32_t index_count = 0;
  uint32_t index_offset = 0;
  int32_t vertex_offset = 0;
};
