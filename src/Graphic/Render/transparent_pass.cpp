#include "transparent_pass.h"

#include "material_renderer.h"

TransparentPass::TransparentPass(TransparentRenderer* renderer) : transparent_renderer_(renderer) {
}

void TransparentPass::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  // Use new unified command system
  command_cache_.clear();
  transparent_renderer_->Build(packet, GetFilter(), command_cache_);
  transparent_renderer_->Record(frame, command_cache_, packet.main_camera, frame.screen_width, frame.screen_height);
}
