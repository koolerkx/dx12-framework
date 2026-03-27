#pragma once
#include <d3d12.h>

#include <algorithm>
#include <vector>

#include "Command/render_command_list.h"
#include "Frame/render_frame_context.h"
#include "Frame/resolved_draw_command.h"
#include "Framework/Render/frame_packet.h"
#include "draw_command_resolver.h"
#include "resolved_command_grouper.h"

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

  virtual void BuildResolved(const RenderFrameContext& frame,
    const FramePacket& packet,
    RenderLayer target_layer,
    const DrawCommandResolver::ResolveContext& ctx,
    std::vector<ResolvedDrawCommand>& out_commands);

  virtual uint64_t ComputeSortKey(const ResolvedDrawCommand& cmd) const {
    return SortKey::DepthFirst(cmd, false);
  }

  void RecordResolvedCommands(const RenderFrameContext& frame,
    std::vector<ResolvedDrawCommand>& commands,
    const CameraData& camera,
    const LightingConfig& lighting,
    const ShadowConfig& shadow,
    uint32_t screen_width,
    uint32_t screen_height,
    float time);

 private:
  struct FrameSetup {
    RenderCommandList cmd;
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

  void RecordWithExecuteIndirect(RenderCommandList& cmd, const std::vector<ResolvedDrawCommand>& commands, const RenderFrameContext& frame);

  void RecordWithDirectDraws(RenderCommandList& cmd, const std::vector<ResolvedDrawCommand>& commands);
};

class OpaqueRenderer : public MaterialRenderer {
 public:
  uint64_t ComputeSortKey(const ResolvedDrawCommand& cmd) const override {
    return SortKey::MaterialFirst(cmd, true);
  }

  void BuildResolved(const RenderFrameContext& frame,
    const FramePacket& packet,
    RenderLayer target_layer,
    const DrawCommandResolver::ResolveContext& ctx,
    std::vector<ResolvedDrawCommand>& out) override {
    MaterialRenderer::BuildResolved(frame, packet, target_layer, ctx, out);
    ResolvedCommandGrouper::Group(out);
    std::sort(out.begin(), out.end(), [](const ResolvedDrawCommand& a, const ResolvedDrawCommand& b) {
      return SortKey::MaterialFirst(a, true) < SortKey::MaterialFirst(b, true);
    });
  }
};

class TransparentRenderer : public MaterialRenderer {
 public:
  void BuildResolved(const RenderFrameContext& frame,
    const FramePacket& packet,
    RenderLayer target_layer,
    const DrawCommandResolver::ResolveContext& ctx,
    std::vector<ResolvedDrawCommand>& out) override {
    MaterialRenderer::BuildResolved(frame, packet, target_layer, ctx, out);
    std::sort(out.begin(), out.end(), [](const ResolvedDrawCommand& a, const ResolvedDrawCommand& b) {
      return SortKey::DepthFirst(a, false) < SortKey::DepthFirst(b, false);
    });
  }
};

class UiRenderer : public MaterialRenderer {
 public:
  void BuildResolved(const RenderFrameContext& frame,
    const FramePacket& packet,
    RenderLayer target_layer,
    const DrawCommandResolver::ResolveContext& ctx,
    std::vector<ResolvedDrawCommand>& out) override {
    MaterialRenderer::BuildResolved(frame, packet, target_layer, ctx, out);
    ResolvedCommandGrouper::Group(out);
    std::sort(out.begin(), out.end(), [](const ResolvedDrawCommand& a, const ResolvedDrawCommand& b) {
      return SortKey::DepthFirst(a, false) < SortKey::DepthFirst(b, false);
    });
  }
};
