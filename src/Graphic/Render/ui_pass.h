#pragma once
#include <vector>

#include "Frame/draw_command.h"
#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"
#include "material_renderer.h"
#include "render_pass.h"

class UiPass : public IRenderPass {
 public:
  explicit UiPass(UiRenderer* renderer);

  void Execute(const RenderFrameContext& frame, const FramePacket& packet) override;

  const char* GetName() const override {
    return "UI Pass";
  }

  RenderPassFilter GetFilter() const override {
    return RenderPassFilter{.target_layer = RenderLayer::UI, .required_tags = 0, .excluded_tags = 0};
  }

 private:
  UiRenderer* ui_renderer_;
  std::vector<RenderCommand> command_cache_;
};
