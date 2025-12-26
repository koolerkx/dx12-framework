#include "ui_pass.h"

#include "graphic.h"
#include "ui_renderer.h"


UiPass::UiPass(UiRenderer* renderer, Graphic* graphic) : ui_renderer_(renderer), graphic_(graphic) {
}

void UiPass::Execute(const RenderFrameContext& frame, const RenderWorld& world) {
  std::vector<UiDrawPacket> packets;
  ui_renderer_->Build(world, packets);
  ui_renderer_->Record(frame, packets, frame.frame_cb, frame.object_cb, frame.screen_width, frame.screen_height);
}
