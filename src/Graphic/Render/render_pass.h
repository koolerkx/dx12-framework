#pragma once

#include <vector>

#include "Framework/Render/frame_packet.h"
#include "Frame/render_frame_context.h"
#include "Render/render_graph_handle.h"

struct PassSetup {
  std::vector<RenderGraphHandle> resource_writes;
  RenderGraphHandle depth = RenderGraphHandle::Invalid;
  std::vector<RenderGraphHandle> resource_reads;
};

class IRenderPass {
 public:
  virtual ~IRenderPass() = default;

  virtual void Execute(const RenderFrameContext& frame, const FramePacket& packet) = 0;
  virtual const char* GetName() const = 0;

  const PassSetup& GetPassSetup() const {
    return setup_;
  }

  void SetGroupName(const char* name) {
    group_name_ = name;
  }

  const char* GetGroupName() const {
    return group_name_;
  }

 protected:
  PassSetup setup_;
  const char* group_name_ = nullptr;
};
