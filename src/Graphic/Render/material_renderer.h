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

namespace SortKey {
// Key: [RS hash 16-bit | PSO hash 16-bit | Depth 32-bit]
[[nodiscard]] uint64_t MaterialFirst(const DrawCommand& cmd, bool front_to_back);
// Key: [Depth 32-bit | RS hash 16-bit | PSO hash 16-bit]
[[nodiscard]] uint64_t DepthFirst(const DrawCommand& cmd, bool front_to_back);
}  // namespace SortKey

// Helper for extracting sort key from DrawCommandVariant (DEPRECATED)
struct DrawCommandVariantSorter {
  static uint64_t GetSortKey(const DrawCommandVariant& variant, bool front_to_back, bool depth_first = false) {
    return std::visit(
      [&](auto&& cmd) -> uint64_t { return cmd.GetSortKeyWithDepth(cmd.material, cmd.depth, front_to_back, depth_first); }, variant);
  }
};

class MaterialRenderer {
 public:
  MaterialRenderer() = default;
  virtual ~MaterialRenderer() = default;

  virtual void Build(const FramePacket& packet, RenderLayer target_layer, std::vector<DrawCommand>& out_commands) = 0;

  virtual void Record(const RenderFrameContext& frame,
    const std::vector<DrawCommand>& commands,
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
  static void FilterCommands(const FramePacket& packet, RenderLayer target_layer, std::vector<DrawCommand>& out) {
    for (const auto& cmd : packet.commands) {
      if (cmd.layer == target_layer) out.push_back(cmd);
    }
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
  void RecordSingle(RenderCommandList& cmd, const DrawCommand& draw_cmd, const DirectX::XMMATRIX& view_proj);
  void RecordInstanced(RenderCommandList& cmd, const DrawCommand& draw_cmd);

  // DEPRECATED: Recording methods for old command types
  void RecordSingleLegacy(RenderCommandList& cmd, const SingleDrawCommand& draw_cmd, const DirectX::XMMATRIX& view_proj);
  void RecordInstanceLegacy(RenderCommandList& cmd, const InstanceDrawCommand& draw_cmd);
};

class OpaqueRenderer : public MaterialRenderer {
 public:
  OpaqueRenderer() = default;

  void Build(const FramePacket& packet, RenderLayer target_layer, std::vector<DrawCommand>& out_commands) override {
    out_commands.clear();
    FilterCommands(packet, target_layer, out_commands);
    out_commands = DrawCommandAggregator::Aggregate(out_commands);
    std::sort(out_commands.begin(), out_commands.end(), [](const DrawCommand& a, const DrawCommand& b) {
      return SortKey::MaterialFirst(a, true) < SortKey::MaterialFirst(b, true);
    });
  }

  void BuildLegacy(const FramePacket& packet, std::vector<DrawCommandVariant>& out_commands) override {
    out_commands.clear();
    out_commands = packet.opaque_pass;
    SortCommandsLegacy(out_commands, true);
  }
};

class TransparentRenderer : public MaterialRenderer {
 public:
  TransparentRenderer() = default;

  void Build(const FramePacket& packet, RenderLayer target_layer, std::vector<DrawCommand>& out_commands) override {
    out_commands.clear();
    FilterCommands(packet, target_layer, out_commands);
    std::sort(out_commands.begin(), out_commands.end(), [](const DrawCommand& a, const DrawCommand& b) {
      return SortKey::DepthFirst(a, false) < SortKey::DepthFirst(b, false);
    });
  }

  void BuildLegacy(const FramePacket& packet, std::vector<DrawCommandVariant>& out_commands) override {
    out_commands.clear();
    out_commands = packet.transparent_pass;
    SortCommandsLegacy(out_commands, false, true);
  }
};

class UiRenderer : public MaterialRenderer {
 public:
  UiRenderer() = default;

  void Build(const FramePacket& packet, RenderLayer target_layer, std::vector<DrawCommand>& out_commands) override {
    out_commands.clear();
    FilterCommands(packet, target_layer, out_commands);
    out_commands = DrawCommandAggregator::Aggregate(out_commands);
    std::sort(out_commands.begin(), out_commands.end(), [](const DrawCommand& a, const DrawCommand& b) {
      return SortKey::MaterialFirst(a, false) < SortKey::MaterialFirst(b, false);
    });
  }

  void BuildLegacy(const FramePacket& packet, std::vector<DrawCommandVariant>& out_commands) override {
    out_commands.clear();
    out_commands = packet.ui_pass;
    SortCommandsLegacy(out_commands, false);
  }
};
