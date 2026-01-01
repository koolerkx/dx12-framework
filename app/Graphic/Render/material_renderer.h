#pragma once
#include <d3d12.h>

#include <algorithm>
#include <variant>
#include <vector>

#include "Command/render_command_list.h"
#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"

// Forward declarations
class RenderCommandList;

// Helper for extracting sort key from DrawCommandVariant
struct DrawCommandVariantSorter {
  static uint64_t GetSortKey(const DrawCommandVariant& variant, bool front_to_back) {
    return std::visit([&](auto&& cmd) -> uint64_t { return cmd.GetSortKeyWithDepth(cmd.material, cmd.depth, front_to_back); }, variant);
  }
};

// Base renderer with material-based sorting
class MaterialRenderer {
 public:
  MaterialRenderer() = default;
  virtual ~MaterialRenderer() = default;

  // Build draw command list from frame packet
  virtual void Build(const FramePacket& packet, std::vector<DrawCommandVariant>& out_commands) = 0;

  // Record GPU commands with optimized material batching
  virtual void Record(const RenderFrameContext& frame,
    const std::vector<DrawCommandVariant>& commands,
    const CameraData& camera,
    uint32_t screen_width,
    uint32_t screen_height);

 protected:
  // Sort commands to minimize RS/PSO switches
  // Strategy: Sort by [RS hash | PSO hash | depth]
  void SortCommands(std::vector<DrawCommandVariant>& commands, bool front_to_back = true) {
    std::sort(commands.begin(), commands.end(), [&](const DrawCommandVariant& a, const DrawCommandVariant& b) -> bool {
      uint64_t a_key = DrawCommandVariantSorter::GetSortKey(a, front_to_back);
      uint64_t b_key = DrawCommandVariantSorter::GetSortKey(b, front_to_back);
      return a_key < b_key;
    });
  }

 private:
  // Separate recording methods for single and instance draw commands
  void RecordSingle(RenderCommandList& cmd, const SingleDrawCommand& draw_cmd, const DirectX::XMMATRIX& view_proj);

  void RecordInstance(RenderCommandList& cmd, const InstanceDrawCommand& draw_cmd);
};

// Opaque renderer (for solid 3D objects)
class OpaqueRenderer : public MaterialRenderer {
 public:
  OpaqueRenderer() = default;

  void Build(const FramePacket& packet, std::vector<DrawCommandVariant>& out_commands) override {
    out_commands.clear();
    out_commands = packet.opaque_pass;

    // Sort front-to-back for better depth testing
    SortCommands(out_commands, true);
  }
};

// Transparent renderer (for alpha-blended objects)
class TransparentRenderer : public MaterialRenderer {
 public:
  TransparentRenderer() = default;

  void Build(const FramePacket& packet, std::vector<DrawCommandVariant>& out_commands) override {
    out_commands.clear();
    out_commands = packet.transparent_pass;

    // Sort back-to-front for correct alpha blending
    SortCommands(out_commands, false);
  }
};

// UI renderer (for sprites, 2D elements)
class UiRenderer : public MaterialRenderer {
 public:
  UiRenderer() = default;

  void Build(const FramePacket& packet, std::vector<DrawCommandVariant>& out_commands) override {
    out_commands.clear();
    out_commands = packet.ui_pass;

    // Sort back-to-front for correct UI layering (tooltips, dialogs, etc)
    SortCommands(out_commands, false);
  }
};
