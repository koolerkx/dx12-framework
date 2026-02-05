#pragma once
#include <DirectXMath.h>

#include <vector>

#include "Pipeline/material.h"
#include "Pipeline/vertex_types.h"
#include "render_layer.h"

class Mesh;
using SpriteInstanceData = Graphics::Vertex::SpriteInstance;

// Unified draw command (replaces variant)
struct DrawCommand {
  // Common
  const Material* material = nullptr;
  const Mesh* mesh = nullptr;
  MaterialInstance material_instance{};
  float depth = 0.0f;

  // Single draw fields (when instances.empty())
  DirectX::XMFLOAT4X4 world_matrix{};
  DirectX::XMFLOAT4 color{1.0f, 1.0f, 1.0f, 1.0f};
  DirectX::XMFLOAT2 uv_offset{0.0f, 0.0f};
  DirectX::XMFLOAT2 uv_scale{1.0f, 1.0f};

  // Instanced draw fields (when !instances.empty())
  std::vector<SpriteInstanceData> instances;

  [[nodiscard]] bool IsInstanced() const {
    return !instances.empty();
  }

  // Sort key methods (from DrawSortKey)
  [[nodiscard]] uint64_t GetSortKeyForOpaque(bool front_to_back) const;
  [[nodiscard]] uint64_t GetSortKeyForTransparent(bool front_to_back) const;
  [[nodiscard]] uint64_t GetSortKey(bool front_to_back, bool depth_first) const;
};

// Wrapper with layer/tag metadata
struct RenderCommand {
  DrawCommand command;
  RenderLayer layer;
  RenderTagMask tags;
};

// Filter for render passes
struct RenderPassFilter {
  RenderLayer target_layer;
  RenderTagMask required_tags = 0;
  RenderTagMask excluded_tags = 0;

  [[nodiscard]] bool Matches(const RenderCommand& cmd) const;
};
