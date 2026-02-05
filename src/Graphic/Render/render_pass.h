#pragma once

#include <vector>

#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"

class RenderTexture;
class DepthBuffer;

struct PassSetup {
  struct ColorTarget {
    RenderTexture* texture = nullptr;  // nullptr = backbuffer
  };
  struct DepthTarget {
    DepthBuffer* buffer = nullptr;  // nullptr = no depth
  };

  std::vector<ColorTarget> color_targets;
  DepthTarget depth;
  std::vector<RenderTexture*> shader_inputs;
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
