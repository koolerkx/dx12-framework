#include "opaque_pass.h"

#include "opaque_renderer.h"

OpaquePass::OpaquePass(OpaqueRenderer* renderer) : opaque_renderer_(renderer) {
}

void OpaquePass::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  packet_cache_.clear();

  opaque_renderer_->Build(packet, packet_cache_);
  opaque_renderer_->Record(frame, packet_cache_, packet.main_camera, frame.screen_width, frame.screen_height);
}
