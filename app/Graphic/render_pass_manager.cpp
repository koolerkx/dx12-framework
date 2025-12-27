#include "render_pass_manager.h"

#include "Core/utils.h"
#include "ui_pass.h"

void RenderPassManager::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  // Pass 1: Opaque (Future)
  // TODO: implement opaque pass

  // Pass 2: UI
  if (ui_pass_) {
    utils::CommandListEventGroup(frame.command_list, L"UI Pass", [&]() { ui_pass_->Execute(frame, packet); });
  }
}
