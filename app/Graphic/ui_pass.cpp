#include "ui_pass.h"

#include "ui_renderer.h"

UiPass::UiPass(UiRenderer* renderer) : ui_renderer_(renderer) {
}

void UiPass::Execute(const RenderFrameContext& frame, const RenderWorld& world) {
  packet_cache_.clear();

  ui_renderer_->Build(world, packet_cache_);
  ui_renderer_->Record(frame, packet_cache_, frame.screen_width, frame.screen_height);
}
