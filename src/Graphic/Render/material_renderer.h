#pragma once
#include <d3d12.h>

#include <algorithm>
#include <vector>

#include "Command/render_command_list.h"
#include "Frame/draw_command.h"
#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"
#include "Frame/resolved_draw_command.h"
#include "Resource/Mesh/mesh_buffer_pool.h"
#include "bindless_instance_grouper.h"
#include "draw_command_aggregator.h"
#include "draw_command_resolver.h"

class DynamicUploadBuffer;
class RenderCommandList;

namespace SortKey {
// Key: [RS hash 16-bit | PSO hash 16-bit | Depth 32-bit]
template <typename T>
[[nodiscard]] uint64_t MaterialFirst(const T& cmd, bool front_to_back);
// Key: [Depth 32-bit | RS hash 16-bit | PSO hash 16-bit]
template <typename T>
[[nodiscard]] uint64_t DepthFirst(const T& cmd, bool front_to_back);
}  // namespace SortKey

class MaterialRenderer {
 public:
  MaterialRenderer() = default;
  virtual ~MaterialRenderer() = default;

  virtual void Build(const FramePacket& packet,
    RenderLayer target_layer,
    std::vector<DrawCommand>& out_commands,
    DynamicUploadBuffer* instance_allocator = nullptr) = 0;

  virtual void BuildResolved(const FramePacket& packet,
    RenderLayer target_layer,
    const DrawCommandResolver::ResolveContext& ctx,
    std::vector<ResolvedDrawCommand>& out_commands);

  virtual uint64_t ComputeSortKey(const DrawCommand& cmd) const {
    return SortKey::DepthFirst(cmd, false);
  }
  virtual uint64_t ComputeSortKey(const ResolvedDrawCommand& cmd) const {
    return SortKey::DepthFirst(cmd, false);
  }

  virtual void Record(const RenderFrameContext& frame,
    const std::vector<DrawCommand>& commands,
    const CameraData& camera,
    const LightingConfig& lighting,
    const ShadowConfig& shadow,
    uint32_t screen_width,
    uint32_t screen_height,
    float time);

  void RecordResolvedCommands(const RenderFrameContext& frame,
    const std::vector<ResolvedDrawCommand>& commands,
    const CameraData& camera,
    const LightingConfig& lighting,
    const ShadowConfig& shadow,
    uint32_t screen_width,
    uint32_t screen_height,
    float time);

  void RecordMerged(const RenderFrameContext& frame,
    const std::vector<DrawCommand>& old_commands,
    const std::vector<ResolvedDrawCommand>& new_commands,
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
  struct FrameSetup {
    RenderCommandList cmd;
    Matrix4 view_proj;
    const Material* current_material;
  };

  FrameSetup SetupFrameState(const RenderFrameContext& frame,
    const CameraData& camera,
    const LightingConfig& lighting,
    const ShadowConfig& shadow,
    uint32_t screen_width,
    uint32_t screen_height,
    float time,
    const Material* first_material);

  void RecordSingle(RenderCommandList& cmd, const DrawCommand& draw_cmd, const Matrix4& view_proj, bool shadow_enabled);
  void RecordInstanced(RenderCommandList& cmd, const DrawCommand& draw_cmd);
  void RecordStructuredInstanced(RenderCommandList& cmd, const DrawCommand& draw_cmd, const Matrix4& view_proj, bool shadow_enabled);
  void RecordBindlessSingle(
    RenderCommandList& cmd, const DrawCommand& draw_cmd, const Matrix4& view_proj, bool shadow_enabled, MeshBufferPool* pool);
  void RecordBindlessStructuredInstanced(
    RenderCommandList& cmd, const DrawCommand& draw_cmd, const Matrix4& view_proj, bool shadow_enabled, MeshBufferPool* pool);

  void RecordResolved(RenderCommandList& cmd, const ResolvedDrawCommand& draw_cmd, const Matrix4& view_proj);
};

class OpaqueRenderer : public MaterialRenderer {
 public:
  uint64_t ComputeSortKey(const DrawCommand& cmd) const override {
    return SortKey::MaterialFirst(cmd, true);
  }
  uint64_t ComputeSortKey(const ResolvedDrawCommand& cmd) const override {
    return SortKey::MaterialFirst(cmd, true);
  }

  void Build(const FramePacket& packet,
    RenderLayer target_layer,
    std::vector<DrawCommand>& out_commands,
    DynamicUploadBuffer* instance_allocator = nullptr) override {
    out_commands.clear();
    FilterCommands(packet, target_layer, out_commands);
    out_commands = DrawCommandAggregator::Aggregate(out_commands);
    std::sort(out_commands.begin(), out_commands.end(), [](const DrawCommand& a, const DrawCommand& b) {
      return SortKey::MaterialFirst(a, true) < SortKey::MaterialFirst(b, true);
    });
    if (instance_allocator) {
      BindlessInstanceGrouper::Group(out_commands, instance_allocator);
    }
  }

  void BuildResolved(const FramePacket& packet,
    RenderLayer target_layer,
    const DrawCommandResolver::ResolveContext& ctx,
    std::vector<ResolvedDrawCommand>& out) override {
    MaterialRenderer::BuildResolved(packet, target_layer, ctx, out);
    std::sort(out.begin(), out.end(), [](const ResolvedDrawCommand& a, const ResolvedDrawCommand& b) {
      return SortKey::MaterialFirst(a, true) < SortKey::MaterialFirst(b, true);
    });
  }
};

class TransparentRenderer : public MaterialRenderer {
 public:
  void Build(const FramePacket& packet,
    RenderLayer target_layer,
    std::vector<DrawCommand>& out_commands,
    DynamicUploadBuffer* /*instance_allocator*/ = nullptr) override {
    out_commands.clear();
    FilterCommands(packet, target_layer, out_commands);
    std::sort(out_commands.begin(), out_commands.end(), [](const DrawCommand& a, const DrawCommand& b) {
      return SortKey::DepthFirst(a, false) < SortKey::DepthFirst(b, false);
    });
  }

  void BuildResolved(const FramePacket& packet,
    RenderLayer target_layer,
    const DrawCommandResolver::ResolveContext& ctx,
    std::vector<ResolvedDrawCommand>& out) override {
    MaterialRenderer::BuildResolved(packet, target_layer, ctx, out);
    std::sort(out.begin(), out.end(), [](const ResolvedDrawCommand& a, const ResolvedDrawCommand& b) {
      return SortKey::DepthFirst(a, false) < SortKey::DepthFirst(b, false);
    });
  }
};

class UiRenderer : public MaterialRenderer {
 public:
  void Build(const FramePacket& packet,
    RenderLayer target_layer,
    std::vector<DrawCommand>& out_commands,
    DynamicUploadBuffer* /*instance_allocator*/ = nullptr) override {
    out_commands.clear();
    FilterCommands(packet, target_layer, out_commands);
    out_commands = DrawCommandAggregator::Aggregate(out_commands);
    std::sort(out_commands.begin(), out_commands.end(), [](const DrawCommand& a, const DrawCommand& b) {
      return SortKey::DepthFirst(a, false) < SortKey::DepthFirst(b, false);
    });
  }

  void BuildResolved(const FramePacket& packet,
    RenderLayer target_layer,
    const DrawCommandResolver::ResolveContext& ctx,
    std::vector<ResolvedDrawCommand>& out) override {
    MaterialRenderer::BuildResolved(packet, target_layer, ctx, out);
    std::sort(out.begin(), out.end(), [](const ResolvedDrawCommand& a, const ResolvedDrawCommand& b) {
      return SortKey::DepthFirst(a, false) < SortKey::DepthFirst(b, false);
    });
  }
};
