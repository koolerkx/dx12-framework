#pragma once
#include <memory>
#include <vector>

#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"
#include "render_pass.h"

class UiPass;

class RenderPassManager {
 public:
  RenderPassManager() = default;

  void Execute(const RenderFrameContext& frame, const FramePacket& packet);

  void ExecuteScenePasses(const RenderFrameContext& frame, const FramePacket& packet);
  void ExecuteUiPass(const RenderFrameContext& frame, const FramePacket& packet);

  void AddPass(std::unique_ptr<IRenderPass> pass);
  void ClearPasses();

 private:
  std::vector<std::unique_ptr<IRenderPass>> scene_passes_;
  std::vector<std::unique_ptr<IRenderPass>> ui_passes_;
};
