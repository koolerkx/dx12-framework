#include "vignette_pass.h"

#include "Pipeline/shader_descriptors.h"
#include "Render/fullscreen_pass.h"
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

class VignettePass : public FullscreenPass<Graphics::PostProcessVignetteShader> {
 public:
  VignettePass(ID3D12Device* device,
    ShaderManager* shader_manager,
    const PassSetup& pass_setup,
    RenderGraphHandle input_handle,
    const VignetteConfig* config)
      : FullscreenPass(device, shader_manager, pass_setup, {.pso_name = L"Vignette_PSO"}), input_handle_(input_handle), config_(config) {
  }

  const char* GetName() const override {
    return "Vignette Pass";
  }

 protected:
  void SetupConstants(RenderCommandList& cmd, const RenderFrameContext& frame, const FramePacket&) override {
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
  }

 private:
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

  AddPass(
    graph, std::make_unique<VignettePass>(props.context.device, props.context.shader_manager, vignette_setup, input_rt, props.config));
}
