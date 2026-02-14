#include "vignette_pass.h"

#include "Command/render_command_list.h"
#include "Core/types.h"
#include "Framework/Logging/logger.h"
#include "Pipeline/pipeline_state_builder.h"
#include "Pipeline/shader_descriptors.h"
#include "Pipeline/shader_manager.h"
#include "Render/render_graph.h"
#include "Rendering/vignette_config.h"

namespace {

struct VignetteCB {
  uint32_t scene_srv_index;
  float intensity;
  float radius;
  float softness;
  float roundness;
  float aspect_ratio;
  uint32_t _pad0;
  uint32_t _pad1;
  float vignette_color[3];
  uint32_t _pad2;
};
static_assert(sizeof(VignetteCB) == 48);

class VignettePass : public IRenderPass {
 public:
  VignettePass(ID3D12Device* device,
    ShaderManager* shader_manager,
    const PassSetup& pass_setup,
    RenderGraphHandle input_handle,
    const VignetteConfig* config)
      : shader_manager_(shader_manager),
        input_handle_(input_handle),
        config_(config) {
    setup_ = pass_setup;

    auto* vs = shader_manager->GetVertexShader<Graphics::PostProcessVignetteShader>();
    auto* ps = shader_manager->GetPixelShader<Graphics::PostProcessVignetteShader>();
    auto* root_sig = shader_manager->GetRootSignature(Graphics::RSPreset::Standard);

    if (!vs || !ps || !root_sig) {
      Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[VignettePass] Shader load failed");
    } else {
      pipeline_state_ = PipelineStateBuilder()
                          .SetRootSignature(root_sig)
                          .SetVertexShader(vs)
                          .SetPixelShader(ps)
                          .SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM)
                          .SetName(L"Vignette_PSO")
                          .Build(device);
    }
  }

  const char* GetName() const override {
    return "Vignette Pass";
  }

  void Execute(const RenderFrameContext& frame, const FramePacket& packet) override {
    if (!pipeline_state_) return;

    RenderCommandList cmd(frame.command_list, frame.dynamic_allocator, frame.frame_cb, frame.object_cb_allocator);

    frame.command_list->SetPipelineState(pipeline_state_.Get());
    frame.command_list->SetGraphicsRootSignature(shader_manager_->GetRootSignature(Graphics::RSPreset::Standard));

    cmd.BindGlobalSRVTable(frame.global_heap_manager);
    cmd.BindSamplerTable(frame.global_heap_manager);

    FrameCB frame_data = {};
    frame_data.screenSize = {
      static_cast<float>(frame.screen_width),
      static_cast<float>(frame.screen_height),
    };
    cmd.SetFrameConstants(frame_data);

    float aspect_ratio = static_cast<float>(frame.screen_width) / static_cast<float>(frame.screen_height);

    VignetteCB cb_data{
      .scene_srv_index = frame.render_graph->GetSrvIndex(input_handle_),
      .intensity = config_->intensity,
      .radius = config_->radius,
      .softness = config_->softness,
      .roundness = config_->roundness,
      .aspect_ratio = aspect_ratio,
      .vignette_color = {config_->vignette_color[0], config_->vignette_color[1], config_->vignette_color[2]},
    };

    constexpr auto POST_PROCESS_CB = RootSlot::ConstantBuffer::Light;
    cmd.SetConstantBufferOverride(POST_PROCESS_CB, cb_data);

    frame.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    frame.command_list->DrawInstanced(3, 1, 0, 0);
  }

 private:
  ShaderManager* shader_manager_;
  ComPtr<ID3D12PipelineState> pipeline_state_;
  RenderGraphHandle input_handle_;
  const VignetteConfig* config_;
};

}  // namespace

void VignettePassGroup::Build(RenderGraph& graph, RenderGraphHandle input_rt, const VignetteBuildProps& props) {
  vignette_rt_ = graph.CreateRenderTexture({
    .name = "vignette_rt",
    .format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .width = props.context.width,
    .height = props.context.height,
    .device = props.context.device,
  });

  PassSetup vignette_setup;
  vignette_setup.resource_writes = {vignette_rt_};
  vignette_setup.resource_reads = {input_rt};

  AddPass(graph,
    std::make_unique<VignettePass>(
      props.context.device, props.context.shader_manager, vignette_setup, input_rt, props.config));
}
