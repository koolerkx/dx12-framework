#pragma once

#include "Pipeline/shader_descriptors.h"
#include "Render/fullscreen_pass.h"
#include "Render/render_graph.h"

struct NormalViewPassProps {
  ID3D12Device* device;
  ShaderManager* shader_manager;
  PassSetup pass_setup;
  RenderGraphHandle source_handle;
};

class NormalViewPass : public FullscreenPass<Graphics::PostProcessNormalViewShader> {
 public:
  explicit NormalViewPass(const NormalViewPassProps& props)
      : FullscreenPass(props.device, props.shader_manager, props.pass_setup, {.pso_name = L"NormalViewPass_PSO"}),
        source_handle_(props.source_handle) {
  }

  const char* GetName() const override {
    return "Normal View Pass";
  }
  const wchar_t* GetWideName() const override {
    return L"Normal View Pass";
  }

 protected:
  void SetupConstants(RenderCommandList& cmd, const RenderFrameContext& frame, const FramePacket&) override {
    struct NormalViewCB {
      uint32_t src_srv_index;
      uint32_t padding[3] = {};
    } cb_data = {.src_srv_index = frame.render_graph->GetSrvIndex(source_handle_)};

    constexpr auto POST_PROCESS_CB = RootSlot::ConstantBuffer::Light;
    cmd.SetConstantBufferOverride(POST_PROCESS_CB, cb_data);
  }

 private:
  RenderGraphHandle source_handle_;
};
