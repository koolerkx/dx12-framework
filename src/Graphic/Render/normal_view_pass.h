#pragma once

#include <d3d12.h>

#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"
#include "Render/render_graph_handle.h"
#include "Render/render_pass.h"

struct ID3D12Device;
class ShaderManager;

struct NormalViewPassProps {
  ID3D12Device* device;
  ShaderManager* shader_manager;
  PassSetup pass_setup;
  RenderGraphHandle source_handle;
};

class NormalViewPass : public IRenderPass {
 public:
  explicit NormalViewPass(const NormalViewPassProps& props);

  const char* GetName() const override {
    return "Normal View Pass";
  }

  void Execute(const RenderFrameContext& frame, const FramePacket& packet) override;

 private:
  bool CreatePipelineObjects();

  ID3D12Device* device_;
  ShaderManager* shader_manager_;
  ComPtr<ID3D12PipelineState> pipeline_state_;
  RenderGraphHandle source_handle_ = RenderGraphHandle::Invalid;
};
