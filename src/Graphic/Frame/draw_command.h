#pragma once
#include <array>
#include <vector>

#include "Framework/Math/Math.h"
#include "Framework/Render/render_handles.h"
#include "Framework/Render/render_types.h"
#include "Pipeline/material.h"
#include "Pipeline/vertex_types.h"

using Math::Matrix4;
using Math::Vector2;
using Math::Vector4;

class Mesh;
using SpriteInstanceData = Graphics::Vertex::SpriteInstance;

struct DrawCommand {
  const Material* material = nullptr;
  const Mesh* mesh = nullptr;
  MeshHandle mesh_handle;
  MaterialHandle material_handle;
  float depth = 0.0f;

  Matrix4 world_matrix{};
  Vector4 color{1.0f, 1.0f, 1.0f, 1.0f};
  Vector2 uv_offset{0.0f, 0.0f};
  Vector2 uv_scale{1.0f, 1.0f};

  std::vector<SpriteInstanceData> instances;

  D3D12_GPU_VIRTUAL_ADDRESS instance_buffer_address = 0;
  uint32_t instance_count = 0;

  std::array<float, 20> custom_data{};
  bool has_custom_data = false;

  RenderLayer layer = RenderLayer::Opaque;
  RenderTagMask tags = 0;
  bool depth_test = true;
  bool depth_write = true;

  [[nodiscard]] bool UsesBindlessMesh() const {
    return mesh_handle.IsValid();
  }

  [[nodiscard]] bool UsesBindlessMaterial() const {
    return material_handle.IsValid();
  }

  [[nodiscard]] bool IsInstanced() const {
    return !instances.empty();
  }

  [[nodiscard]] bool IsStructuredInstanced() const {
    return instance_count > 0 && instance_buffer_address != 0;
  }
};
