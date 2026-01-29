#include "transparent_pass.h"

#include "material_renderer.h"

TransparentPass::TransparentPass(TransparentRenderer* renderer) : transparent_renderer_(renderer) {
}

void TransparentPass::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  packet_cache_.clear();

  transparent_renderer_->Build(packet, packet_cache_);
  transparent_renderer_->Record(frame, packet_cache_, packet.main_camera, frame.screen_width, frame.screen_height);
}
