#pragma once
#include <d3d12.h>

#include <algorithm>
#include <variant>
#include <vector>

#include "Command/render_command_list.h"
#include "Frame/draw_command.h"
#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"
#include "draw_command_aggregator.h"

// Forward declarations
class RenderCommandList;

// Helper for extracting sort key from DrawCommandVariant (DEPRECATED)
struct DrawCommandVariantSorter {
  static uint64_t GetSortKey(const DrawCommandVariant& variant, bool front_to_back, bool depth_first = false) {
    return std::visit(
      [&](auto&& cmd) -> uint64_t { return cmd.GetSortKeyWithDepth(cmd.material, cmd.depth, front_to_back, depth_first); }, variant);
  }
};

// Base renderer with material-based sorting
class MaterialRenderer {
 public:
  MaterialRenderer() = default;
  virtual ~MaterialRenderer() = default;

  // NEW: Build with filter from unified commands
  virtual void Build(const FramePacket& packet, const RenderPassFilter& filter, std::vector<RenderCommand>& out_commands) = 0;

  // NEW: Record with unified commands
  virtual void Record(const RenderFrameContext& frame,
    const std::vector<RenderCommand>& commands,
    const CameraData& camera,
    uint32_t screen_width,
    uint32_t screen_height);

  // DEPRECATED: Build from old pass arrays
  virtual void BuildLegacy(const FramePacket& packet, std::vector<DrawCommandVariant>& out_commands) = 0;

  // DEPRECATED: Record with variant commands
  virtual void RecordLegacy(const RenderFrameContext& frame,
    const std::vector<DrawCommandVariant>& commands,
    const CameraData& camera,
    uint32_t screen_width,
    uint32_t screen_height);

 protected:
  // NEW: Sort RenderCommand list
  void SortCommands(std::vector<RenderCommand>& commands, bool front_to_back = true, bool depth_first = false) {
    std::sort(commands.begin(), commands.end(), [&](const RenderCommand& a, const RenderCommand& b) -> bool {
      uint64_t a_key = a.command.GetSortKey(front_to_back, depth_first);
      uint64_t b_key = b.command.GetSortKey(front_to_back, depth_first);
      return a_key < b_key;
    });
  }

  // DEPRECATED: Sort DrawCommandVariant list
  void SortCommandsLegacy(std::vector<DrawCommandVariant>& commands, bool front_to_back = true, bool depth_first = false) {
    std::sort(commands.begin(), commands.end(), [&](const DrawCommandVariant& a, const DrawCommandVariant& b) -> bool {
      uint64_t a_key = DrawCommandVariantSorter::GetSortKey(a, front_to_back, depth_first);
      uint64_t b_key = DrawCommandVariantSorter::GetSortKey(b, front_to_back, depth_first);
      return a_key < b_key;
    });
  }

 private:
  // NEW: Recording methods for unified DrawCommand
  void RecordSingle(RenderCommandList& cmd, const DrawCommand& draw_cmd, const DirectX::XMMATRIX& view_proj);
  void RecordInstanced(RenderCommandList& cmd, const DrawCommand& draw_cmd);

  // DEPRECATED: Recording methods for old command types
  void RecordSingleLegacy(RenderCommandList& cmd, const SingleDrawCommand& draw_cmd, const DirectX::XMMATRIX& view_proj);
  void RecordInstanceLegacy(RenderCommandList& cmd, const InstanceDrawCommand& draw_cmd);
};

// Opaque renderer (for solid 3D objects)
class OpaqueRenderer : public MaterialRenderer {
 public:
  OpaqueRenderer() = default;

  void Build(const FramePacket& packet, const RenderPassFilter& filter, std::vector<RenderCommand>& out_commands) override {
    out_commands.clear();

    // Filter commands by layer/tag
    for (const auto& cmd : packet.commands) {
      if (filter.Matches(cmd)) {
        out_commands.push_back(cmd);
      }
    }

    // Aggregate compatible commands
    out_commands = DrawCommandAggregator::Aggregate(out_commands);

    // Sort front-to-back for better depth testing
    SortCommands(out_commands, true);
  }

  void BuildLegacy(const FramePacket& packet, std::vector<DrawCommandVariant>& out_commands) override {
    out_commands.clear();
    out_commands = packet.opaque_pass;
    SortCommandsLegacy(out_commands, true);
  }
};

// Transparent renderer (for alpha-blended objects)
class TransparentRenderer : public MaterialRenderer {
 public:
  TransparentRenderer() = default;

  void Build(const FramePacket& packet, const RenderPassFilter& filter, std::vector<RenderCommand>& out_commands) override {
    out_commands.clear();

    // Filter commands by layer/tag
    for (const auto& cmd : packet.commands) {
      if (filter.Matches(cmd)) {
        out_commands.push_back(cmd);
      }
    }

    // Skip aggregation for transparent pass (depth order matters)
    // Sort back-to-front for correct alpha blending
    SortCommands(out_commands, false, true);
  }

  void BuildLegacy(const FramePacket& packet, std::vector<DrawCommandVariant>& out_commands) override {
    out_commands.clear();
    out_commands = packet.transparent_pass;
    SortCommandsLegacy(out_commands, false, true);
  }
};

// UI renderer (for sprites, 2D elements)
class UiRenderer : public MaterialRenderer {
 public:
  UiRenderer() = default;

  void Build(const FramePacket& packet, const RenderPassFilter& filter, std::vector<RenderCommand>& out_commands) override {
    out_commands.clear();

    // Filter commands by layer/tag
    for (const auto& cmd : packet.commands) {
      if (filter.Matches(cmd)) {
        out_commands.push_back(cmd);
      }
    }

    // Aggregate compatible commands for UI (same layer_id can batch)
    out_commands = DrawCommandAggregator::Aggregate(out_commands);

    // Sort back-to-front for correct UI layering
    SortCommands(out_commands, false);
  }

  void BuildLegacy(const FramePacket& packet, std::vector<DrawCommandVariant>& out_commands) override {
    out_commands.clear();
    out_commands = packet.ui_pass;
    SortCommandsLegacy(out_commands, false);
  }
};
