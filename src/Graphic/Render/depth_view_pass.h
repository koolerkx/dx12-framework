#pragma once

#include <d3d12.h>

#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"
#include "Render/render_graph_handle.h"
#include "Render/render_pass.h"
#include "Rendering/hdr_config.h"

struct ID3D12Device;
class ShaderManager;

struct DepthViewPassProps {
  ID3D12Device* device;
  ShaderManager* shader_manager;
  PassSetup pass_setup;
  RenderGraphHandle depth_handle;
  const DepthViewConfig* config;
};

class DepthViewPass : public IRenderPass {
 public:
  explicit DepthViewPass(const DepthViewPassProps& props);
  ~DepthViewPass() = default;

  const char* GetName() const override {
    return "Depth View Pass";
  }

  void Execute(const RenderFrameContext& frame, const FramePacket& packet) override;

 private:
  bool CreatePipelineObjects();

  ID3D12Device* device_;
  ShaderManager* shader_manager_;
  ComPtr<ID3D12PipelineState> pipeline_state_;

  RenderGraphHandle depth_handle_ = RenderGraphHandle::Invalid;
  const DepthViewConfig* config_ = nullptr;
};
