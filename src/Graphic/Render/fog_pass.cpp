#include "fog_pass.h"

#include "Pipeline/shader_descriptors.h"
#include "Render/fullscreen_pass.h"
#include "Render/render_graph.h"
#include "Rendering/post_process_config.h"

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

class FogPass : public FullscreenPass<Graphics::PostProcessFogShader> {
 public:
  FogPass(ID3D12Device* device,
    ShaderManager* shader_manager,
    const PassSetup& pass_setup,
    RenderGraphHandle input_handle,
    RenderGraphHandle normal_depth_handle,
    const FogConfig* config)
      : FullscreenPass(device, shader_manager, pass_setup, {.pso_name = L"Fog_PSO"}),
        input_handle_(input_handle),
        normal_depth_handle_(normal_depth_handle),
        config_(config) {
  }

  const char* GetName() const override {
    return "Fog Pass";
  }

 protected:
  void SetupConstants(RenderCommandList& cmd, const RenderFrameContext& frame, const FramePacket& packet) override {
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
  }

 private:
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
