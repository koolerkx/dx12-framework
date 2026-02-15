#include "fog_pass.h"

#include "Command/render_command_list.h"
#include "Core/types.h"
#include "Framework/Logging/logger.h"
#include "Pipeline/pipeline_state_builder.h"
#include "Pipeline/shader_descriptors.h"
#include "Pipeline/shader_manager.h"
#include "Render/render_graph.h"
#include "Rendering/fog_config.h"

namespace {

struct FogCB {
  uint32_t scene_srv_index;
  uint32_t depth_srv_index;
  float density;
  float height_falloff;
  float base_height;
  float max_distance;
  uint32_t _pad0;
  uint32_t _pad1;
  float fog_color[3];
  uint32_t _pad2;
};
static_assert(sizeof(FogCB) == 48);

class FogPass : public IRenderPass {
 public:
  FogPass(ID3D12Device* device,
    ShaderManager* shader_manager,
    const PassSetup& pass_setup,
    RenderGraphHandle input_handle,
    RenderGraphHandle normal_depth_handle,
    const FogConfig* config)
      : shader_manager_(shader_manager), input_handle_(input_handle), normal_depth_handle_(normal_depth_handle), config_(config) {
    setup_ = pass_setup;

    auto* vs = shader_manager->GetVertexShader<Graphics::PostProcessFogShader>();
    auto* ps = shader_manager->GetPixelShader<Graphics::PostProcessFogShader>();
    auto* root_sig = shader_manager->GetRootSignature(Graphics::RSPreset::Standard);

    if (!vs || !ps || !root_sig) {
      Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[FogPass] Shader load failed");
    } else {
      pipeline_state_ = PipelineStateBuilder()
                          .SetRootSignature(root_sig)
                          .SetVertexShader(vs)
                          .SetPixelShader(ps)
                          .SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM)
                          .SetName(L"Fog_PSO")
                          .Build(device);
    }
  }

  const char* GetName() const override {
    return "Fog Pass";
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

    FogCB cb_data{
      .scene_srv_index = frame.render_graph->GetSrvIndex(input_handle_),
      .depth_srv_index = frame.render_graph->GetSrvIndex(normal_depth_handle_),
      .density = config_->density,
      .height_falloff = config_->height_falloff,
      .base_height = config_->base_height,
      .max_distance = config_->max_distance,
      .fog_color = {config_->fog_color[0], config_->fog_color[1], config_->fog_color[2]},
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
  RenderGraphHandle normal_depth_handle_;
  const FogConfig* config_;
};

}  // namespace

void FogPassGroup::Build(RenderGraph& graph, RenderGraphHandle input_rt, RenderGraphHandle normal_depth_rt, const FogBuildProps& props) {
  fog_rt_ = graph.CreateRenderTexture({
    .name = "fog_rt",
    .format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .width = props.context.width,
    .height = props.context.height,
    .device = props.context.device,
  });

  PassSetup fog_setup;
  fog_setup.resource_writes = {fog_rt_};
  fog_setup.resource_reads = {input_rt, normal_depth_rt};

  AddPass(graph,
    std::make_unique<FogPass>(props.context.device, props.context.shader_manager, fog_setup, input_rt, normal_depth_rt, props.config));
}
