#pragma once
#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"

class IRenderPass {
 public:
  virtual ~IRenderPass() = default;

  virtual void Execute(const RenderFrameContext& frame, const FramePacket& packet) = 0;

  virtual const char* GetName() const = 0;
};
