#pragma once
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

 private:
  UiRenderer* ui_renderer_;
  std::vector<DrawCommand> packet_cache_;
};
