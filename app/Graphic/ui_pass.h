#pragma once
#include "render_frame_context.h"
#include "render_world.h"


class UiRenderer;
class Graphic;

class UiPass {
 public:
  explicit UiPass(UiRenderer* renderer, Graphic* graphic);
  void Execute(const RenderFrameContext& frame, const RenderWorld& world);

 private:
  UiRenderer* ui_renderer_;
  Graphic* graphic_;
};
