#pragma once
#include "frame_packet.h"
#include "render_frame_context.h"
#include "ui_renderer.h"

class Graphic;

class UiPass {
 public:
  explicit UiPass(UiRenderer* renderer);

  void Execute(const RenderFrameContext& frame, const FramePacket& packet);

 private:
  UiRenderer* ui_renderer_;
  std::vector<UiDrawCommand> packet_cache_;
};
