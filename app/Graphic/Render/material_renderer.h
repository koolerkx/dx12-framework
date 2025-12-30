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
  float depth = 0.0f;  // For depth sorting within same material

  // Combined sort key for optimizing RS/PSO switches
  // Material's sort key already contains [RS hash | PSO hash]
  // We use the full 64-bit key for primary sorting
  uint64_t GetSortKey() const {
    if (!material) {
      return UINT64_MAX;
    }
    // Return material's 64-bit sort key directly
    // High 32 bits: RS hash (primary sort)
    // Low 32 bits: PSO hash (secondary sort)
    return material->GetSortKey();
  }

  // Get sort key with depth incorporated (for depth-based sorting within same material)
  uint64_t GetSortKeyWithDepth(bool front_to_back = true) const {
    if (!material) {
      return UINT64_MAX;
    }

    // Convert depth to uint32 for bitwise sorting
    uint32_t depth_key = std::bit_cast<uint32_t>(depth);

    // For back-to-front, invert depth bits
    if (!front_to_back) {
      depth_key = ~depth_key;
    }

    // Combine: [RS hash 16-bit | PSO hash 16-bit | Depth 32-bit]
    // To fit depth, we compress RS and PSO keys to 16-bit each
    uint32_t rs_key = material->GetRootSignatureKey() & 0xFFFF;
    uint32_t pso_key = material->GetPSOKey() & 0xFFFF;
    uint64_t compressed_material = (static_cast<uint64_t>(rs_key) << 48) | (static_cast<uint64_t>(pso_key) << 32);

    return compressed_material | depth_key;
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
  // Sort commands to minimize RS/PSO switches
  // Strategy: Sort by [RS hash | PSO hash | depth]
  void SortCommands(std::vector<DrawCommand>& commands, bool front_to_back = true) {
    if (front_to_back) {
      // For opaque geometry: front-to-back (early depth test optimization)
      // Sort by material key first (RS+PSO), then by depth within same material
      std::sort(commands.begin(), commands.end(), [](const DrawCommand& a, const DrawCommand& b) {
        uint64_t a_mat = a.GetSortKey();
        uint64_t b_mat = b.GetSortKey();

        // Primary sort: by material (RS + PSO)
        if (a_mat != b_mat) {
          return a_mat < b_mat;
        }

        // Secondary sort: by depth (front-to-back)
        return a.depth < b.depth;
      });
    } else {
      // For transparent geometry: back-to-front (correct alpha blending)
      // Still group by material, but reverse depth order
      std::sort(commands.begin(), commands.end(), [](const DrawCommand& a, const DrawCommand& b) {
        uint64_t a_mat = a.GetSortKey();
        uint64_t b_mat = b.GetSortKey();

        // Primary sort: by material (RS + PSO)
        if (a_mat != b_mat) {
          return a_mat < b_mat;
        }

        // Secondary sort: by depth (back-to-front)
        return a.depth > b.depth;
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

    // Sort by material first (minimize PSO switches), then by depth (back-to-front)
    // This ensures proper UI layering (tooltips, dialogs, etc)
    std::sort(out_commands.begin(), out_commands.end(), [](const DrawCommand& a, const DrawCommand& b) {
      uint64_t a_mat = a.GetSortKey();
      uint64_t b_mat = b.GetSortKey();

      // Primary: group by material for PSO batching
      if (a_mat != b_mat) {
        return a_mat < b_mat;
      }

      // Secondary: back-to-front for correct UI layering
      // Higher depth values render last (on top)
      return a.depth > b.depth;
    });
  }
};
