#pragma once
#include "Frame/draw_command.h"
#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"
#include "render_pass.h"

// Forward declarations
class DebugLineRenderer;
class MaterialManager;

class DebugPass : public IRenderPass {
 public:
  DebugPass(DebugLineRenderer* renderer, MaterialManager* material_mgr);
  void Execute(const RenderFrameContext& frame, const FramePacket& packet) override;

  const char* GetName() const override {
    return "Debug Pass";
  }

  RenderPassFilter GetFilter() const override {
    return RenderPassFilter{.target_layer = RenderLayer::Debug, .required_tags = 0, .excluded_tags = 0};
  }

 private:
  DebugLineRenderer* debug_line_renderer_;
  MaterialManager* material_manager_;
};
