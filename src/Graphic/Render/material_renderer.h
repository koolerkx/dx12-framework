#pragma once
#include <d3d12.h>

#include <algorithm>
#include <vector>

#include "Command/render_command_list.h"
#include "Frame/draw_command.h"
#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"
#include "Resource/Mesh/mesh_buffer_pool.h"
#include "draw_command_aggregator.h"


class RenderCommandList;

namespace SortKey {
// Key: [RS hash 16-bit | PSO hash 16-bit | Depth 32-bit]
[[nodiscard]] uint64_t MaterialFirst(const DrawCommand& cmd, bool front_to_back);
// Key: [Depth 32-bit | RS hash 16-bit | PSO hash 16-bit]
[[nodiscard]] uint64_t DepthFirst(const DrawCommand& cmd, bool front_to_back);
}  // namespace SortKey

class MaterialRenderer {
 public:
  MaterialRenderer() = default;
  virtual ~MaterialRenderer() = default;

  virtual void Build(const FramePacket& packet, RenderLayer target_layer, std::vector<DrawCommand>& out_commands) = 0;

  virtual void Record(const RenderFrameContext& frame,
    const std::vector<DrawCommand>& commands,
    const CameraData& camera,
    const LightingConfig& lighting,
    const ShadowConfig& shadow,
    uint32_t screen_width,
    uint32_t screen_height,
    float time);

 protected:
  static void FilterCommands(const FramePacket& packet, RenderLayer target_layer, std::vector<DrawCommand>& out) {
    for (const auto& cmd : packet.commands) {
      if (cmd.layer == target_layer) out.push_back(cmd);
    }
  }

 private:
  void RecordSingle(RenderCommandList& cmd, const DrawCommand& draw_cmd, const Matrix4& view_proj, bool shadow_enabled);
  void RecordInstanced(RenderCommandList& cmd, const DrawCommand& draw_cmd);
  void RecordStructuredInstanced(RenderCommandList& cmd, const DrawCommand& draw_cmd, const Matrix4& view_proj, bool shadow_enabled);
  void RecordBindlessSingle(
    RenderCommandList& cmd, const DrawCommand& draw_cmd, const Matrix4& view_proj, bool shadow_enabled, MeshBufferPool* pool);
  void RecordBindlessStructuredInstanced(
    RenderCommandList& cmd, const DrawCommand& draw_cmd, const Matrix4& view_proj, bool shadow_enabled, MeshBufferPool* pool);
};

class OpaqueRenderer : public MaterialRenderer {
 public:
  void Build(const FramePacket& packet, RenderLayer target_layer, std::vector<DrawCommand>& out_commands) override {
    out_commands.clear();
    FilterCommands(packet, target_layer, out_commands);
    out_commands = DrawCommandAggregator::Aggregate(out_commands);
    std::sort(out_commands.begin(), out_commands.end(), [](const DrawCommand& a, const DrawCommand& b) {
      return SortKey::MaterialFirst(a, true) < SortKey::MaterialFirst(b, true);
    });
  }
};

class TransparentRenderer : public MaterialRenderer {
 public:
  void Build(const FramePacket& packet, RenderLayer target_layer, std::vector<DrawCommand>& out_commands) override {
    out_commands.clear();
    FilterCommands(packet, target_layer, out_commands);
    std::sort(out_commands.begin(), out_commands.end(), [](const DrawCommand& a, const DrawCommand& b) {
      return SortKey::DepthFirst(a, false) < SortKey::DepthFirst(b, false);
    });
  }
};

class UiRenderer : public MaterialRenderer {
 public:
  void Build(const FramePacket& packet, RenderLayer target_layer, std::vector<DrawCommand>& out_commands) override {
    out_commands.clear();
    FilterCommands(packet, target_layer, out_commands);
    out_commands = DrawCommandAggregator::Aggregate(out_commands);
    std::sort(out_commands.begin(), out_commands.end(), [](const DrawCommand& a, const DrawCommand& b) {
      return SortKey::DepthFirst(a, false) < SortKey::DepthFirst(b, false);
    });
  }
};
