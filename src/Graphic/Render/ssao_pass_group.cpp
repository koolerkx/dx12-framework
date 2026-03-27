#include "ssao_pass_group.h"

#include "Pipeline/shader_descriptors.h"
#include "Render/fullscreen_pass.h"
#include "Render/render_graph.h"
#include "Rendering/post_process_config.h"

namespace {

struct SSAOCB {
  uint32_t normal_depth_srv_index;
  float radius;
  float bias;
  float intensity;
  uint32_t sample_count;
  uint32_t _pad[3];
};
static_assert(sizeof(SSAOCB) == 32);

struct SSAOBlurCB {
  uint32_t ao_srv_index;
  uint32_t normal_depth_srv_index;
  float texel_size_x;
  float texel_size_y;
};
static_assert(sizeof(SSAOBlurCB) == 16);

class SSAOPass : public FullscreenPass<Graphics::PostProcessSSAOShader> {
 public:
  SSAOPass(ID3D12Device* device,
    ShaderManager* shader_manager,
    const PassSetup& pass_setup,
    RenderGraphHandle normal_depth_handle,
    const SSAOConfig* config)
      : FullscreenPass(device, shader_manager, pass_setup, {.rt_format = DXGI_FORMAT_R8_UNORM, .pso_name = L"SSAO_PSO"}),
        normal_depth_handle_(normal_depth_handle),
        config_(config) {
  }

  const char* GetName() const override {
    return "SSAO Pass";
  }
  const wchar_t* GetWideName() const override {
    return L"SSAO Pass";
  }

 protected:
  void SetupConstants(RenderCommandList& cmd, const RenderFrameContext& frame, const FramePacket& packet) override {
    FrameCB frame_data = {};
    frame_data.proj = packet.main_camera.proj;
    frame_data.invProj = packet.main_camera.inv_proj;
    cmd.SetFrameConstants(frame_data);

    SSAOCB cb_data{
      .normal_depth_srv_index = frame.render_graph->GetSrvIndex(normal_depth_handle_),
      .radius = config_->radius,
      .bias = config_->bias,
      .intensity = config_->intensity,
      .sample_count = config_->sample_count,
    };

    constexpr auto POST_PROCESS_CB = RootSlot::ConstantBuffer::Light;
    cmd.SetConstantBufferOverride(POST_PROCESS_CB, cb_data);
  }

 private:
  RenderGraphHandle normal_depth_handle_;
  const SSAOConfig* config_;
};

enum class BlurDirection : uint8_t { Horizontal, Vertical };

class SSAOBlurPass : public FullscreenPass<Graphics::PostProcessSSAOBlurShader> {
 public:
  SSAOBlurPass(ID3D12Device* device,
    ShaderManager* shader_manager,
    const PassSetup& pass_setup,
    RenderGraphHandle ao_handle,
    RenderGraphHandle normal_depth_handle,
    BlurDirection direction)
      : FullscreenPass(device,
          shader_manager,
          pass_setup,
          {.rt_format = DXGI_FORMAT_R8_UNORM, .pso_name = direction == BlurDirection::Horizontal ? L"SSAOBlurH_PSO" : L"SSAOBlurV_PSO"}),
        ao_handle_(ao_handle),
        normal_depth_handle_(normal_depth_handle),
        direction_(direction) {
  }

  const char* GetName() const override {
    return direction_ == BlurDirection::Horizontal ? "SSAO Blur H Pass" : "SSAO Blur V Pass";
  }

 protected:
  void SetupConstants(RenderCommandList& cmd, const RenderFrameContext& frame, const FramePacket&) override {
    auto [w, h] = frame.render_graph->GetTextureSize(ao_handle_);
    float inv_w = 1.0f / static_cast<float>(w);
    float inv_h = 1.0f / static_cast<float>(h);

    SSAOBlurCB cb_data{
      .ao_srv_index = frame.render_graph->GetSrvIndex(ao_handle_),
      .normal_depth_srv_index = frame.render_graph->GetSrvIndex(normal_depth_handle_),
      .texel_size_x = direction_ == BlurDirection::Horizontal ? inv_w : 0.0f,
      .texel_size_y = direction_ == BlurDirection::Vertical ? inv_h : 0.0f,
    };

    constexpr auto POST_PROCESS_CB = RootSlot::ConstantBuffer::Light;
    cmd.SetConstantBufferOverride(POST_PROCESS_CB, cb_data);
  }

 private:
  RenderGraphHandle ao_handle_;
  RenderGraphHandle normal_depth_handle_;
  BlurDirection direction_;
};

}  // namespace

void SSAOPassGroup::Build(RenderGraph& graph, RenderGraphHandle normal_depth_rt, const SSAOBuildProps& props) {
  // SSAO focus on low frequency, half resolution help improve performance while not much quality is lost
  uint32_t half_width = props.context.width / 2;
  uint32_t half_height = props.context.height / 2;
  constexpr float HALF_RES_SCALE = 0.5f;

  auto raw_ao = graph.CreateRenderTexture({
    .name = "ssao_raw_rt",
    .format = DXGI_FORMAT_R8_UNORM,
    .width = half_width,
    .height = half_height,
    .device = props.context.device,
    .clear_color = colors::White,
    .scale_factor = HALF_RES_SCALE,
  });

  // separate horizontal and vertical blur to improve performance
  auto blur_h_rt = graph.CreateRenderTexture({
    .name = "ssao_blur_h_rt",
    .format = DXGI_FORMAT_R8_UNORM,
    .width = half_width,
    .height = half_height,
    .device = props.context.device,
    .clear_color = colors::White,
    .scale_factor = HALF_RES_SCALE,
  });

  blurred_ao_ = graph.CreateRenderTexture({
    .name = "ssao_blurred_rt",
    .format = DXGI_FORMAT_R8_UNORM,
    .width = half_width,
    .height = half_height,
    .device = props.context.device,
    .clear_color = colors::White,
    .scale_factor = HALF_RES_SCALE,
  });

  PassSetup ssao_setup;
  ssao_setup.resource_writes = {raw_ao};
  ssao_setup.resource_reads = {normal_depth_rt};

  AddPass(graph, std::make_unique<SSAOPass>(props.context.device, props.context.shader_manager, ssao_setup, normal_depth_rt, props.config));

  PassSetup blur_h_setup;
  blur_h_setup.resource_writes = {blur_h_rt};
  blur_h_setup.resource_reads = {raw_ao, normal_depth_rt};

  AddPass(graph,
    std::make_unique<SSAOBlurPass>(
      props.context.device, props.context.shader_manager, blur_h_setup, raw_ao, normal_depth_rt, BlurDirection::Horizontal));

  PassSetup blur_v_setup;
  blur_v_setup.resource_writes = {blurred_ao_};
  blur_v_setup.resource_reads = {blur_h_rt, normal_depth_rt};

  AddPass(graph,
    std::make_unique<SSAOBlurPass>(
      props.context.device, props.context.shader_manager, blur_v_setup, blur_h_rt, normal_depth_rt, BlurDirection::Vertical));
}
