#pragma once
#include "render_frame_context.h"
#include "render_world.h"

class UiPass;

class RenderPassManager {
 public:
  void Execute(const RenderFrameContext& frame, const RenderWorld& world);

  void SetUiPass(UiPass* pass) {
    ui_pass_ = pass;
  }

 private:
  UiPass* ui_pass_ = nullptr;
};
