#include "ssao_pass_group.h"

#include "Command/render_command_list.h"
#include "Core/types.h"
#include "Framework/Logging/logger.h"
#include "Pipeline/pipeline_state_builder.h"
#include "Pipeline/shader_descriptors.h"
#include "Pipeline/shader_manager.h"
#include "Render/render_graph.h"
#include "Rendering/ssao_config.h"

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

class SSAOPass : public IRenderPass {
 public:
  SSAOPass(ID3D12Device* device,
    ShaderManager* shader_manager,
    const PassSetup& pass_setup,
    RenderGraphHandle normal_depth_handle,
    const SSAOConfig* config)
      : shader_manager_(shader_manager), normal_depth_handle_(normal_depth_handle), config_(config) {
    setup_ = pass_setup;

    auto* vs = shader_manager->GetVertexShader<Graphics::PostProcessSSAOShader>();
    auto* ps = shader_manager->GetPixelShader<Graphics::PostProcessSSAOShader>();
    auto* root_sig = shader_manager->GetRootSignature(Graphics::RSPreset::Standard);

    if (!vs || !ps || !root_sig) {
      Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[SSAOPass] Shader load failed");
    } else {
      pipeline_state_ = PipelineStateBuilder()
                          .SetRootSignature(root_sig)
                          .SetVertexShader(vs)
                          .SetPixelShader(ps)
                          .SetRenderTargetFormat(DXGI_FORMAT_R8_UNORM)
                          .SetName(L"SSAO_PSO")
                          .Build(device);
    }
  }

  const char* GetName() const override {
    return "SSAO Pass";
  }

  void Execute(const RenderFrameContext& frame, const FramePacket& packet) override {
    if (!pipeline_state_) return;

    RenderCommandList cmd(frame.command_list, frame.dynamic_allocator, frame.frame_cb, frame.object_cb_allocator);

    frame.command_list->SetPipelineState(pipeline_state_.Get());
    frame.command_list->SetGraphicsRootSignature(shader_manager_->GetRootSignature(Graphics::RSPreset::Standard));

    cmd.BindGlobalSRVTable(frame.global_heap_manager);
    cmd.BindSamplerTable(frame.global_heap_manager);

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

    frame.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    frame.command_list->DrawInstanced(3, 1, 0, 0);
  }

 private:
  ShaderManager* shader_manager_;
  ComPtr<ID3D12PipelineState> pipeline_state_;
  RenderGraphHandle normal_depth_handle_;
  const SSAOConfig* config_;
};

class SSAOBlurPass : public IRenderPass {
 public:
  SSAOBlurPass(ID3D12Device* device,
    ShaderManager* shader_manager,
    const PassSetup& pass_setup,
    RenderGraphHandle ao_handle,
    RenderGraphHandle normal_depth_handle)
      : shader_manager_(shader_manager), ao_handle_(ao_handle), normal_depth_handle_(normal_depth_handle) {
    setup_ = pass_setup;

    auto* vs = shader_manager->GetVertexShader<Graphics::PostProcessSSAOBlurShader>();
    auto* ps = shader_manager->GetPixelShader<Graphics::PostProcessSSAOBlurShader>();
    auto* root_sig = shader_manager->GetRootSignature(Graphics::RSPreset::Standard);

    if (!vs || !ps || !root_sig) {
      Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[SSAOBlurPass] Shader load failed");
    } else {
      pipeline_state_ = PipelineStateBuilder()
                          .SetRootSignature(root_sig)
                          .SetVertexShader(vs)
                          .SetPixelShader(ps)
                          .SetRenderTargetFormat(DXGI_FORMAT_R8_UNORM)
                          .SetName(L"SSAOBlur_PSO")
                          .Build(device);
    }
  }

  const char* GetName() const override {
    return "SSAO Blur Pass";
  }

  void Execute(const RenderFrameContext& frame, const FramePacket&) override {
    if (!pipeline_state_) return;

    RenderCommandList cmd(frame.command_list, frame.dynamic_allocator, frame.frame_cb, frame.object_cb_allocator);

    frame.command_list->SetPipelineState(pipeline_state_.Get());
    frame.command_list->SetGraphicsRootSignature(shader_manager_->GetRootSignature(Graphics::RSPreset::Standard));

    cmd.BindGlobalSRVTable(frame.global_heap_manager);
    cmd.BindSamplerTable(frame.global_heap_manager);

    auto [w, h] = frame.render_graph->GetTextureSize(ao_handle_);

    SSAOBlurCB cb_data{
      .ao_srv_index = frame.render_graph->GetSrvIndex(ao_handle_),
      .normal_depth_srv_index = frame.render_graph->GetSrvIndex(normal_depth_handle_),
      .texel_size_x = 1.0f / static_cast<float>(w),
      .texel_size_y = 1.0f / static_cast<float>(h),
    };

    constexpr auto POST_PROCESS_CB = RootSlot::ConstantBuffer::Light;
    cmd.SetConstantBufferOverride(POST_PROCESS_CB, cb_data);

    frame.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    frame.command_list->DrawInstanced(3, 1, 0, 0);
  }

 private:
  ShaderManager* shader_manager_;
  ComPtr<ID3D12PipelineState> pipeline_state_;
  RenderGraphHandle ao_handle_;
  RenderGraphHandle normal_depth_handle_;
};

}  // namespace

void SSAOPassGroup::Build(RenderGraph& graph, RenderGraphHandle normal_depth_rt, const SSAOBuildProps& props) {
  auto raw_ao = graph.CreateRenderTexture({
    .name = "ssao_raw_rt",
    .format = DXGI_FORMAT_R8_UNORM,
    .width = props.context.width,
    .height = props.context.height,
    .device = props.context.device,
    .clear_color = colors::White,
  });

  blurred_ao_ = graph.CreateRenderTexture({
    .name = "ssao_blurred_rt",
    .format = DXGI_FORMAT_R8_UNORM,
    .width = props.context.width,
    .height = props.context.height,
    .device = props.context.device,
    .clear_color = colors::White,
  });

  PassSetup ssao_setup;
  ssao_setup.resource_writes = {raw_ao};
  ssao_setup.resource_reads = {normal_depth_rt};

  AddPass(graph, std::make_unique<SSAOPass>(
    props.context.device, props.context.shader_manager, ssao_setup, normal_depth_rt, props.config));

  PassSetup blur_setup;
  blur_setup.resource_writes = {blurred_ao_};
  blur_setup.resource_reads = {raw_ao, normal_depth_rt};

  AddPass(graph, std::make_unique<SSAOBlurPass>(
    props.context.device, props.context.shader_manager, blur_setup, raw_ao, normal_depth_rt));
}
