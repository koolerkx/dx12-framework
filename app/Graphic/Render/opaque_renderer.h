#pragma once
#include <d3d12.h>

#include <vector>

#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"

class OpaqueRenderer {
 public:
  OpaqueRenderer(ID3D12RootSignature* root_sig, ID3D12PipelineState* pso);

  void Build(const FramePacket& packet, std::vector<OpaqueDrawCommand>& out_cache);

  void Record(const RenderFrameContext& frame,
    const std::vector<OpaqueDrawCommand>& commands,
    const CameraData& camera,
    uint32_t screen_width,
    uint32_t screen_height);

 private:
  ID3D12RootSignature* root_signature_;
  ID3D12PipelineState* pipeline_state_;
};
