#pragma once
#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"

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
