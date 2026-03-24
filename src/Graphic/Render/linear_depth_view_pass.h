#pragma once

#include "Pipeline/shader_descriptors.h"
#include "Render/fullscreen_pass.h"
#include "Render/render_graph.h"
#include "Rendering/post_process_config.h"

struct LinearDepthViewPassProps {
  ID3D12Device* device;
  ShaderManager* shader_manager;
  PassSetup pass_setup;
  RenderGraphHandle source_handle;
  const DepthViewConfig* config;
};

class LinearDepthViewPass : public FullscreenPass<Graphics::PostProcessLinearDepthViewShader> {
 public:
  explicit LinearDepthViewPass(const LinearDepthViewPassProps& props)
      : FullscreenPass(props.device, props.shader_manager, props.pass_setup,
                        {.pso_name = L"LinearDepthViewPass_PSO"}),
        source_handle_(props.source_handle),
        config_(props.config) {}

  const char* GetName() const override { return "Linear Depth View Pass"; }

 protected:
  bool ShouldExecute(const RenderFrameContext&, const FramePacket&) const override {
    return pipeline_state_ && config_->enabled;
  }

  void SetupConstants(RenderCommandList& cmd, const RenderFrameContext& frame,
                      const FramePacket&) override {
    struct LinearDepthViewCB {
      uint32_t src_srv_index;
      float near_plane;
      float far_plane;
      uint32_t padding = 0;
    } cb_data = {
      .src_srv_index = frame.render_graph->GetSrvIndex(source_handle_),
      .near_plane = config_->near_plane,
      .far_plane = config_->far_plane,
    };

    constexpr auto POST_PROCESS_CB = RootSlot::ConstantBuffer::Light;
    cmd.SetConstantBufferOverride(POST_PROCESS_CB, cb_data);
  }

 private:
  RenderGraphHandle source_handle_;
  const DepthViewConfig* config_;
};
