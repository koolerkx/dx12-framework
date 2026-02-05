#pragma once
#include "Frame/draw_command.h"
#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"

class IRenderPass {
 public:
  virtual ~IRenderPass() = default;

  virtual void Execute(const RenderFrameContext& frame, const FramePacket& packet) = 0;

  virtual const char* GetName() const = 0;

  // Get filter for this pass (used by unified command system)
  virtual RenderPassFilter GetFilter() const = 0;

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
