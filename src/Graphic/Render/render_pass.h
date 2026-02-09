#pragma once

#include <vector>

#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"
#include "Render/render_graph_handle.h"

struct PassSetup {
  std::vector<RenderGraphHandle> color_targets;
  RenderGraphHandle depth = RenderGraphHandle::Invalid;
  std::vector<RenderGraphHandle> shader_inputs;
};

class IRenderPass {
 public:
  virtual ~IRenderPass() = default;

  virtual void Execute(const RenderFrameContext& frame, const FramePacket& packet) = 0;
  virtual const char* GetName() const = 0;

  const PassSetup& GetPassSetup() const {
    return setup_;
  }

 protected:
  PassSetup setup_;
};
