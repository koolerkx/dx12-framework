#pragma once
#include <vector>

#include "Frame/draw_command.h"
#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"
#include "material_renderer.h"
#include "render_pass.h"

class TransparentPass : public IRenderPass {
 public:
  explicit TransparentPass(TransparentRenderer* renderer);

  void Execute(const RenderFrameContext& frame, const FramePacket& packet) override;

  const char* GetName() const override {
    return "Transparent Pass";
  }

  RenderPassFilter GetFilter() const override {
    return RenderPassFilter{.target_layer = RenderLayer::Transparent, .required_tags = 0, .excluded_tags = 0};
  }

 private:
  TransparentRenderer* transparent_renderer_;
  std::vector<RenderCommand> command_cache_;
};
