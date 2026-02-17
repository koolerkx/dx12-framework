#include "chromatic_aberration_pass.h"

#include "Pipeline/shader_descriptors.h"
#include "Render/fullscreen_pass.h"
#include "Render/render_graph.h"
#include "Rendering/chromatic_aberration_config.h"

namespace {

struct ChromaticAberrationCB {
  uint32_t scene_srv_index;
  float intensity;
  uint32_t _pad0;
  uint32_t _pad1;
};
static_assert(sizeof(ChromaticAberrationCB) == 16);

class ChromaticAberrationPass : public FullscreenPass<Graphics::PostProcessChromaticAberrationShader> {
 public:
  ChromaticAberrationPass(ID3D12Device* device,
    ShaderManager* shader_manager,
    const PassSetup& pass_setup,
    RenderGraphHandle input_handle,
    const ChromaticAberrationConfig* config)
      : FullscreenPass(device, shader_manager, pass_setup, {.pso_name = L"ChromaticAberration_PSO"}),
        input_handle_(input_handle),
        config_(config) {
  }

  const char* GetName() const override {
    return "Chromatic Aberration Pass";
  }

 protected:
  void SetupConstants(RenderCommandList& cmd, const RenderFrameContext& frame, const FramePacket&) override {
    FrameCB frame_data = {};
    frame_data.screenSize = {
      static_cast<float>(frame.screen_width),
      static_cast<float>(frame.screen_height),
    };
    cmd.SetFrameConstants(frame_data);

    ChromaticAberrationCB cb_data{
      .scene_srv_index = frame.render_graph->GetSrvIndex(input_handle_),
      .intensity = config_->intensity,
    };

    constexpr auto POST_PROCESS_CB = RootSlot::ConstantBuffer::Light;
    cmd.SetConstantBufferOverride(POST_PROCESS_CB, cb_data);
  }

 private:
  RenderGraphHandle input_handle_;
  const ChromaticAberrationConfig* config_;
};

}  // namespace

void ChromaticAberrationPassGroup::Build(RenderGraph& graph, RenderGraphHandle input_rt, const ChromaticAberrationBuildProps& props) {
  chromatic_aberration_rt_ = graph.CreateRenderTexture({
    .name = "chromatic_aberration_rt",
    .format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .width = props.context.width,
    .height = props.context.height,
    .device = props.context.device,
  });

  PassSetup setup;
  setup.resource_writes = {chromatic_aberration_rt_};
  setup.resource_reads = {input_rt};

  AddPass(
    graph, std::make_unique<ChromaticAberrationPass>(props.context.device, props.context.shader_manager, setup, input_rt, props.config));
}
