#include "outline_pass.h"

#include "Command/render_command_list.h"
#include "Core/types.h"
#include "Framework/Logging/logger.h"
#include "Pipeline/pipeline_state_builder.h"
#include "Pipeline/shader_descriptors.h"
#include "Pipeline/shader_manager.h"
#include "Render/render_graph.h"
#include "Rendering/outline_config.h"

namespace {

struct OutlineCB {
  uint32_t normal_depth_srv_index;
  uint32_t scene_srv_index;
  float depth_weight;
  float normal_weight;
  float edge_threshold;
  float depth_falloff;
  float thickness;
  uint32_t _pad0;
  float outline_color[3];
  uint32_t _pad1;
};
static_assert(sizeof(OutlineCB) == 48);

class OutlinePass : public IRenderPass {
 public:
  OutlinePass(ID3D12Device* device,
    ShaderManager* shader_manager,
    const PassSetup& pass_setup,
    RenderGraphHandle tonemap_handle,
    RenderGraphHandle normal_depth_handle,
    const OutlineConfig* config)
      : shader_manager_(shader_manager),
        tonemap_handle_(tonemap_handle),
        normal_depth_handle_(normal_depth_handle),
        config_(config) {
    setup_ = pass_setup;

    auto* vs = shader_manager->GetVertexShader<Graphics::PostProcessOutlineShader>();
    auto* ps = shader_manager->GetPixelShader<Graphics::PostProcessOutlineShader>();
    auto* root_sig = shader_manager->GetRootSignature(Graphics::RSPreset::Standard);

    if (!vs || !ps || !root_sig) {
      Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[OutlinePass] Shader load failed");
    } else {
      pipeline_state_ = PipelineStateBuilder()
                          .SetRootSignature(root_sig)
                          .SetVertexShader(vs)
                          .SetPixelShader(ps)
                          .SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM)
                          .SetName(L"Outline_PSO")
                          .Build(device);
    }
  }

  const char* GetName() const override {
    return "Outline Pass";
  }

  void Execute(const RenderFrameContext& frame, const FramePacket& packet) override {
    if (!pipeline_state_) return;

    RenderCommandList cmd(frame.command_list, frame.dynamic_allocator, frame.frame_cb, frame.object_cb_allocator);

    frame.command_list->SetPipelineState(pipeline_state_.Get());
    frame.command_list->SetGraphicsRootSignature(shader_manager_->GetRootSignature(Graphics::RSPreset::Standard));

    cmd.BindGlobalSRVTable(frame.global_heap_manager);
    cmd.BindSamplerTable(frame.global_heap_manager);

    FrameCB frame_data = {};
    frame_data.invView = packet.main_camera.inv_view;
    frame_data.invProj = packet.main_camera.inv_proj;
    frame_data.screenSize = {
      static_cast<float>(frame.screen_width),
      static_cast<float>(frame.screen_height),
    };
    frame_data.cameraPos = packet.main_camera.position;
    cmd.SetFrameConstants(frame_data);

    OutlineCB cb_data{
      .normal_depth_srv_index = frame.render_graph->GetSrvIndex(normal_depth_handle_),
      .scene_srv_index = frame.render_graph->GetSrvIndex(tonemap_handle_),
      .depth_weight = config_->depth_weight,
      .normal_weight = config_->normal_weight,
      .edge_threshold = config_->edge_threshold,
      .depth_falloff = config_->depth_falloff,
      .thickness = config_->thickness,
      .outline_color = {config_->outline_color[0], config_->outline_color[1], config_->outline_color[2]},
    };

    constexpr auto POST_PROCESS_CB = RootSlot::ConstantBuffer::Light;
    cmd.SetConstantBufferOverride(POST_PROCESS_CB, cb_data);

    frame.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    frame.command_list->DrawInstanced(3, 1, 0, 0);
  }

 private:
  ShaderManager* shader_manager_;
  ComPtr<ID3D12PipelineState> pipeline_state_;
  RenderGraphHandle tonemap_handle_;
  RenderGraphHandle normal_depth_handle_;
  const OutlineConfig* config_;
};

}  // namespace

void OutlinePassGroup::Build(RenderGraph& graph, RenderGraphHandle tonemap_rt, RenderGraphHandle normal_depth_rt, const OutlineBuildProps& props) {
  outline_rt_ = graph.CreateRenderTexture({
    .name = "outline_rt",
    .format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .width = props.context.width,
    .height = props.context.height,
    .device = props.context.device,
  });

  PassSetup outline_setup;
  outline_setup.resource_writes = {outline_rt_};
  outline_setup.resource_reads = {tonemap_rt, normal_depth_rt};

  AddPass(graph,
    std::make_unique<OutlinePass>(
      props.context.device, props.context.shader_manager, outline_setup, tonemap_rt, normal_depth_rt, props.config));
}
