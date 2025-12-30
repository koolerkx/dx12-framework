#pragma once
#include <d3d12.h>

#include <algorithm>
#include <bit>
#include <vector>

#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"
#include "Pipeline/material.h"

// Generic draw command with material
struct DrawCommand {
  const Material* material = nullptr;
  const Mesh* mesh = nullptr;
  DirectX::XMFLOAT4X4 world_matrix;
  DirectX::XMFLOAT4 color;
  MaterialInstance material_instance;
  float depth = 0.0f;  // For depth sorting

  // Combined sort key for optimizing PSO/material switches
  uint64_t GetSortKey() const {
    // High 32 bits: material sort key (groups by PSO)
    // Low 32 bits: depth for front-to-back sorting within same material
    uint64_t material_key = material ? material->GetSortKey() : UINT32_MAX;
    // uint32_t depth_key = *reinterpret_cast<const uint32_t*>(&depth);  // Reinterpret float as uint for sorting
    uint32_t depth_key = std::bit_cast<uint32_t>(depth);
    return (material_key << 32) | depth_key;
  }
};

// Base renderer with material-based sorting
class MaterialRenderer {
 public:
  MaterialRenderer() = default;
  virtual ~MaterialRenderer() = default;

  // Build draw command list from frame packet
  virtual void Build(const FramePacket& packet, std::vector<DrawCommand>& out_commands) = 0;

  // Record GPU commands with optimized material batching
  virtual void Record(const RenderFrameContext& frame,
    const std::vector<DrawCommand>& commands,
    const CameraData& camera,
    uint32_t screen_width,
    uint32_t screen_height);

 protected:
  // Sort commands to minimize PSO switches (sort by material first, then depth)
  void SortCommands(std::vector<DrawCommand>& commands, bool front_to_back = true) {
    if (front_to_back) {
      // For opaque geometry: front-to-back (early depth test optimization)
      std::sort(
        commands.begin(), commands.end(), [](const DrawCommand& a, const DrawCommand& b) { return a.GetSortKey() < b.GetSortKey(); });
    } else {
      // For transparent geometry: back-to-front (correct alpha blending)
      std::sort(commands.begin(), commands.end(), [](const DrawCommand& a, const DrawCommand& b) {
        uint64_t a_key = a.GetSortKey();
        uint64_t b_key = b.GetSortKey();
        // Same material group, but reverse depth order
        uint32_t a_mat = static_cast<uint32_t>(a_key >> 32);
        uint32_t b_mat = static_cast<uint32_t>(b_key >> 32);
        if (a_mat != b_mat) {
          return a_mat < b_mat;  // Sort by material first
        }
        return a.depth > b.depth;  // Then back-to-front
      });
    }
  }
};

// Opaque renderer (for solid 3D objects)
class OpaqueRenderer : public MaterialRenderer {
 public:
  OpaqueRenderer() = default;

  void Build(const FramePacket& packet, std::vector<DrawCommand>& out_commands) override {
    out_commands.clear();

    // Convert opaque draw commands to generic draw commands
    for (const auto& opaque_cmd : packet.opaque_pass) {
      DrawCommand cmd;
      cmd.material = opaque_cmd.material;
      cmd.mesh = opaque_cmd.mesh;
      cmd.world_matrix = opaque_cmd.world_matrix;
      cmd.color = opaque_cmd.color;
      cmd.material_instance = opaque_cmd.material_instance;
      cmd.depth = opaque_cmd.depth;
      out_commands.push_back(cmd);
    }

    // Sort front-to-back for better depth testing
    SortCommands(out_commands, true);
  }
};

// Transparent renderer (for alpha-blended objects)
class TransparentRenderer : public MaterialRenderer {
 public:
  TransparentRenderer() = default;

  void Build(const FramePacket& packet, std::vector<DrawCommand>& out_commands) override {
    out_commands.clear();

    // Convert transparent draw commands
    for (const auto& transparent_cmd : packet.transparent_pass) {
      DrawCommand cmd;
      cmd.material = transparent_cmd.material;
      cmd.mesh = transparent_cmd.mesh;
      cmd.world_matrix = transparent_cmd.world_matrix;
      cmd.color = transparent_cmd.color;
      cmd.material_instance = transparent_cmd.material_instance;
      cmd.depth = transparent_cmd.depth;
      out_commands.push_back(cmd);
    }

    // Sort back-to-front for correct alpha blending
    SortCommands(out_commands, false);
  }
};

// UI renderer (for sprites, 2D elements)
class UiRenderer : public MaterialRenderer {
 public:
  UiRenderer() = default;

  void Build(const FramePacket& packet, std::vector<DrawCommand>& out_commands) override {
    out_commands.clear();

    // Convert UI draw commands
    for (const auto& ui_cmd : packet.ui_pass) {
      DrawCommand cmd;
      cmd.material = ui_cmd.material;
      cmd.mesh = ui_cmd.mesh;

      // For UI rendering, we need to transform the world matrix to work with orthographic projection
      // The world matrix from transform contains position/scale/rotation in world space
      // But UI needs screen-space coordinates
      // We'll keep the world matrix as-is and let the orthographic projection handle the mapping
      cmd.world_matrix = ui_cmd.world_matrix;

      cmd.color = ui_cmd.color;
      cmd.material_instance = ui_cmd.material_instance;
      cmd.depth = ui_cmd.depth;
      out_commands.push_back(cmd);
    }

    // UI typically renders back-to-front (painter's algorithm)
    SortCommands(out_commands, false);
  }
};
