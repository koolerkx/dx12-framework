#pragma once

#include "Frame/render_frame_context.h"
#include "Framework/Render/frame_packet.h"
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
  const wchar_t* GetWideName() const override {
    return L"Debug Pass";
  }

 private:
  DebugLineRenderer* debug_line_renderer_;
  MaterialManager* material_manager_;
};
