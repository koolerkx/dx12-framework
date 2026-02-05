#pragma once

#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"
#include "render_pass.h"

class DebugLineRenderer;
class MaterialManager;

class DebugPass : public IRenderPass {
 public:
  DebugPass(DebugLineRenderer* renderer, MaterialManager* material_mgr, PassSetup pass_setup);

  void Execute(const RenderFrameContext& frame, const FramePacket& packet) override;

  const char* GetName() const override {
    return "Debug Pass";
  }

 private:
  DebugLineRenderer* debug_line_renderer_;
  MaterialManager* material_manager_;
};
