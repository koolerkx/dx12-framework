#pragma once
#include <vector>

#include "Frame/draw_command.h"
#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"
#include "material_renderer.h"
#include "render_pass.h"

class OpaquePass : public IRenderPass {
 public:
  explicit OpaquePass(OpaqueRenderer* renderer);

  void Execute(const RenderFrameContext& frame, const FramePacket& packet) override;

  const char* GetName() const override {
    return "Opaque Pass";
  }

  RenderPassFilter GetFilter() const override {
    return RenderPassFilter{.target_layer = RenderLayer::Opaque, .required_tags = 0, .excluded_tags = 0};
  }

 private:
  OpaqueRenderer* opaque_renderer_;
  std::vector<RenderCommand> command_cache_;
  std::vector<DrawCommandVariant> legacy_cache_;
};
