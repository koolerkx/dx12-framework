#pragma once
#include <d3d12.h>

#include "Framework/Math/Math.h"
#include "Framework/Render/render_handles.h"
#include "Framework/Render/render_request.h"
#include "Framework/Render/render_types.h"
#include "Graphic/Frame/mesh_geometry.h"

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

  RenderLayer layer = RenderLayer::Opaque;
  RenderTagMask tags = 0;
  float depth = 0.0f;

  CustomShaderData custom_data;
};
