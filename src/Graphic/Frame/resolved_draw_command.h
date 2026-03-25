#pragma once
#include <d3d12.h>

#include <cstdint>

#include "Framework/Math/Math.h"
#include "Framework/Render/render_handles.h"
#include "Framework/Render/render_request.h"
#include "Framework/Render/render_types.h"

struct MeshGeometry {
  D3D12_VERTEX_BUFFER_VIEW vbv{};
  D3D12_INDEX_BUFFER_VIEW ibv{};
  uint32_t index_count = 0;
  uint32_t index_offset = 0;
  int32_t vertex_offset = 0;
};

class Material;

struct ResolvedDrawCommand {
  const Material* material = nullptr;
  MeshGeometry geometry;

  Math::Matrix4 world_matrix;
  Math::Vector4 color{1, 1, 1, 1};
  Math::Vector2 uv_offset{0, 0};
  Math::Vector2 uv_scale{1, 1};

  uint32_t object_flags = 0;
  uint32_t instance_count = 1;
  D3D12_GPU_VIRTUAL_ADDRESS instance_buffer_address = 0;
  MaterialHandle material_handle;
  uint32_t object_index = UINT32_MAX;

  RenderLayer layer = RenderLayer::Opaque;
  RenderTagMask tags = 0;
  float depth = 0.0f;
  bool depth_write = true;

  CustomShaderData custom_data;
};
