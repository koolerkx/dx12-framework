#pragma once
#include "render_frame_context.h"
#include "render_world.h"
#include "ui_renderer.h"

class UiRenderer;
class Graphic;

class UiPass {
 public:
  explicit UiPass(UiRenderer* renderer);
  void Execute(const RenderFrameContext& frame, const RenderWorld& world);

 private:
  UiRenderer* ui_renderer_;
  std::vector<UiDrawPacket> packet_cache_;
};
