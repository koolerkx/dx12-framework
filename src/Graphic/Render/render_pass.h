#pragma once
#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"

class IRenderPass {
 public:
  virtual ~IRenderPass() = default;

  virtual void Execute(const RenderFrameContext& frame, const FramePacket& packet) = 0;

  virtual const char* GetName() const = 0;

  // Optional callbacks for state transitions before/after pass execution
  virtual void PreExecute(const RenderFrameContext& frame, const FramePacket& packet) {
    (void)frame;
    (void)packet;
  }

  virtual void PostExecute(const RenderFrameContext& frame, const FramePacket& packet) {
    (void)frame;
    (void)packet;
  }
};
