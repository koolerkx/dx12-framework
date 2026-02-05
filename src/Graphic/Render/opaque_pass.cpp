#include "opaque_pass.h"

#include "material_renderer.h"

OpaquePass::OpaquePass(OpaqueRenderer* renderer) : opaque_renderer_(renderer) {
}

void OpaquePass::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  // Use new unified command system
  command_cache_.clear();
  opaque_renderer_->Build(packet, GetFilter(), command_cache_);
  opaque_renderer_->Record(frame, command_cache_, packet.main_camera, frame.screen_width, frame.screen_height);
}
