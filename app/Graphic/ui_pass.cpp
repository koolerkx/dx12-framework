#include "ui_pass.h"

#include "ui_renderer.h"

UiPass::UiPass(UiRenderer* renderer) : ui_renderer_(renderer) {
}

void UiPass::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  // Clear cache for safety, though Build usually clears it
  packet_cache_.clear();

  // Extract and Sort UI commands from the FramePacket
  ui_renderer_->Build(packet, packet_cache_);

  // Execute Draw Calls
  ui_renderer_->Record(frame, packet_cache_, frame.screen_width, frame.screen_height);
}
