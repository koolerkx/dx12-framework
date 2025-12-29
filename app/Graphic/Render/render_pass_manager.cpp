#include "render_pass_manager.h"

#include "Core/utils.h"

void RenderPassManager::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  for (auto& pass : passes_) {
    if (pass) {
      utils::CommandListEventGroup(
        frame.command_list, utils::utf8_to_wstring(pass->GetName()).c_str(), [&]() { pass->Execute(frame, packet); });
    }
  }
}

void RenderPassManager::AddPass(std::unique_ptr<IRenderPass> pass) {
  passes_.push_back(std::move(pass));
}

void RenderPassManager::ClearPasses() {
  passes_.clear();
}
