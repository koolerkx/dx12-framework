#include "render_pass_manager.h"

#include "Core/utils.h"
#include "Render/ui_pass.h"

void RenderPassManager::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  // Legacy: Execute all passes for backward compatibility
  ExecuteScenePasses(frame, packet);
  ExecuteUiPass(frame, packet);
}

void RenderPassManager::ExecuteScenePasses(const RenderFrameContext& frame, const FramePacket& packet) {
  for (auto& pass : scene_passes_) {
    if (pass) {
      utils::CommandListEventGroup(frame.command_list, utils::utf8_to_wstring(pass->GetName()).c_str(), [&]() {
        pass->PreExecute(frame, packet);
        pass->Execute(frame, packet);
        pass->PostExecute(frame, packet);
      });
    }
  }
}

void RenderPassManager::ExecuteUiPass(const RenderFrameContext& frame, const FramePacket& packet) {
  for (auto& pass : ui_passes_) {
    if (pass) {
      utils::CommandListEventGroup(frame.command_list, utils::utf8_to_wstring(pass->GetName()).c_str(), [&]() {
        pass->PreExecute(frame, packet);
        pass->Execute(frame, packet);
        pass->PostExecute(frame, packet);
      });
    }
  }
}

void RenderPassManager::AddPass(std::unique_ptr<IRenderPass> pass) {
  // Auto-categorize: UI pass goes to ui_passes_, others to scene_passes_
  if (dynamic_cast<UiPass*>(pass.get()) != nullptr) {
    ui_passes_.push_back(std::move(pass));
  } else {
    scene_passes_.push_back(std::move(pass));
  }
}

void RenderPassManager::ClearPasses() {
  scene_passes_.clear();
  ui_passes_.clear();
}
