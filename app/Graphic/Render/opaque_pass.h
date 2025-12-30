#pragma once
#include <vector>

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

 private:
  OpaqueRenderer* opaque_renderer_;
  std::vector<DrawCommand> packet_cache_;
};
