#pragma once

#include <d3d12.h>

#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"
#include "Render/render_graph_handle.h"
#include "Render/render_pass.h"
#include "Rendering/hdr_config.h"

struct ID3D12Device;
class ShaderManager;

struct LinearDepthViewPassProps {
  ID3D12Device* device;
  ShaderManager* shader_manager;
  PassSetup pass_setup;
  RenderGraphHandle source_handle;
  const DepthViewConfig* config;
};

class LinearDepthViewPass : public IRenderPass {
 public:
  explicit LinearDepthViewPass(const LinearDepthViewPassProps& props);

  const char* GetName() const override {
    return "Linear Depth View Pass";
  }

  void Execute(const RenderFrameContext& frame, const FramePacket& packet) override;

 private:
  bool CreatePipelineObjects();

  ID3D12Device* device_;
  ShaderManager* shader_manager_;
  ComPtr<ID3D12PipelineState> pipeline_state_;
  RenderGraphHandle source_handle_ = RenderGraphHandle::Invalid;
  const DepthViewConfig* config_ = nullptr;
};
