#include "opaque_pass.h"

#include "material_renderer.h"

OpaquePass::OpaquePass(OpaqueRenderer* renderer) : opaque_renderer_(renderer) {
}

void OpaquePass::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  command_cache_.clear();
  opaque_renderer_->Build(packet, RenderLayer::Opaque, command_cache_);
  opaque_renderer_->Record(frame, command_cache_, packet.main_camera, frame.screen_width, frame.screen_height);
}
