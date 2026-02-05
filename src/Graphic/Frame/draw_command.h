#pragma once
#include <vector>

#include "Framework/Math/Math.h"
#include "Pipeline/material.h"
#include "Pipeline/vertex_types.h"
#include "render_layer.h"

using Math::Matrix4;
using Math::Vector2;
using Math::Vector4;

class Mesh;
using SpriteInstanceData = Graphics::Vertex::SpriteInstance;

struct DrawCommand {
  const Material* material = nullptr;
  const Mesh* mesh = nullptr;
  MaterialInstance material_instance{};
  float depth = 0.0f;

  Matrix4 world_matrix{};
  Vector4 color{1.0f, 1.0f, 1.0f, 1.0f};
  Vector2 uv_offset{0.0f, 0.0f};
  Vector2 uv_scale{1.0f, 1.0f};

  std::vector<SpriteInstanceData> instances;

  RenderLayer layer = RenderLayer::Opaque;
  RenderTagMask tags = 0;

  [[nodiscard]] bool IsInstanced() const {
    return !instances.empty();
  }
};
