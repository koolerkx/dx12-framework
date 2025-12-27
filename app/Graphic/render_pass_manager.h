#pragma once
#include "frame_packet.h"
#include "render_frame_context.h"

class UiPass;

class RenderPassManager {
 public:
  void Execute(const RenderFrameContext& frame, const FramePacket& packet);

  void SetUiPass(UiPass* pass) {
    ui_pass_ = pass;
  }

 private:
  UiPass* ui_pass_ = nullptr;
};
