#pragma once
#include <d3d12.h>

#include <cstdint>
#include <vector>

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

  MeshHandle mesh;
  uint32_t object_flags = 0;
  uint32_t instance_count = 1;
  MaterialHandle material_handle;
  uint32_t object_index = UINT32_MAX;
  uint32_t start_instance_location = 0;
  std::vector<uint32_t> grouped_object_indices;

  RenderLayer layer = RenderLayer::Opaque;
  RenderTagMask tags = 0;
  float depth = 0.0f;
  bool depth_write = true;

  bool IsDrawable() const {
    return material && geometry.index_count > 0;
  }

  CustomShaderData custom_data;
};
