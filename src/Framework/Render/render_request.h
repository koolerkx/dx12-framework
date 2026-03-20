#pragma once
#include <array>
#include <cstdint>

#include "Framework/Math/Math.h"
#include "Framework/Render/render_handles.h"
#include "Framework/Render/render_settings.h"
#include "Framework/Render/render_types.h"

struct CustomShaderData {
  std::array<float, 20> data{};
  bool active = false;
};

struct RenderRequest {
  MeshHandle mesh;
  MaterialHandle material;
  uint32_t shader_id = 0;
  Rendering::RenderSettings render_settings;

  Math::Matrix4 world_matrix;
  Math::Vector4 color{1, 1, 1, 1};
  Math::Vector2 uv_offset{0, 0};
  Math::Vector2 uv_scale{1, 1};

  RenderLayer layer = RenderLayer::Opaque;
  RenderTagMask tags = 0;
  float depth = 0.0f;

  CustomShaderData custom_data;
};

struct InstancedRenderRequest {
  MeshHandle mesh;
  MaterialHandle material;
  uint32_t shader_id = 0;
  Rendering::RenderSettings render_settings;

  Math::Vector4 color{1, 1, 1, 1};

  RenderLayer layer = RenderLayer::Opaque;
  RenderTagMask tags = 0;
  float depth = 0.0f;

  CustomShaderData custom_data;
};
