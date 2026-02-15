#pragma once

#include "Pipeline/shader_descriptors.h"
#include "Render/fullscreen_pass.h"
#include "Render/render_graph.h"

struct BlitPassProps {
  ID3D12Device* device;
  ShaderManager* shader_manager;
  PassSetup pass_setup;
  RenderGraphHandle source_handle;
};

class BlitPass : public FullscreenPass<Graphics::PostProcessBlitShader> {
 public:
  explicit BlitPass(const BlitPassProps& props)
      : FullscreenPass(props.device, props.shader_manager, props.pass_setup, {.pso_name = L"BlitPass_PSO"}),
        source_handle_(props.source_handle) {
  }

  const char* GetName() const override {
    return "Blit Pass";
  }

 protected:
  void SetupConstants(RenderCommandList& cmd, const RenderFrameContext& frame, const FramePacket&) override {
    struct BlitCB {
      uint32_t src_srv_index;
      uint32_t padding[3] = {};
    } cb_data = {.src_srv_index = frame.render_graph->GetSrvIndex(source_handle_)};

    constexpr auto POST_PROCESS_CB = RootSlot::ConstantBuffer::Light;
    cmd.SetConstantBufferOverride(POST_PROCESS_CB, cb_data);
  }

 private:
  RenderGraphHandle source_handle_;
};
