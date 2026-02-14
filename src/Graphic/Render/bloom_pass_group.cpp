#include "bloom_pass_group.h"

#include <algorithm>
#include <format>
#include <vector>

#include "Command/render_command_list.h"
#include "Core/types.h"
#include "Framework/Logging/logger.h"
#include "Pipeline/pipeline_state_builder.h"
#include "Pipeline/shader_descriptors.h"
#include "Pipeline/shader_manager.h"
#include "Render/render_graph.h"
#include "Rendering/hdr_config.h"

namespace {

struct BloomDownCB {
  uint32_t source_srv_index;
  float texel_size_x;
  float texel_size_y;
  float threshold;
};
static_assert(sizeof(BloomDownCB) == 16);

struct BloomUpCB {
  uint32_t source_srv_index;
  float texel_size_x;
  float texel_size_y;
  float padding = 0;
};
static_assert(sizeof(BloomUpCB) == 16);

class BloomDownsamplePass : public IRenderPass {
 public:
  BloomDownsamplePass(ID3D12Device* device,
    ShaderManager* shader_manager,
    const PassSetup& pass_setup,
    RenderGraphHandle source_handle,
    const BloomConfig* config,
    const char* name)
      : shader_manager_(shader_manager), source_handle_(source_handle), config_(config), name_(name) {
    setup_ = pass_setup;

    auto* vs = shader_manager->GetVertexShader<Graphics::PostProcessBloomDownsampleShader>();
    auto* ps = shader_manager->GetPixelShader<Graphics::PostProcessBloomDownsampleShader>();
    auto* root_sig = shader_manager->GetRootSignature(Graphics::RSPreset::Standard);

    if (!vs || !ps || !root_sig) {
      Logger::LogFormat(LogLevel::Error,
        LogCategory::Graphic,
        Logger::Here(),
        "[BloomDownsample] Shader load failed: vs={} ps={} rs={}",
        (void*)vs,
        (void*)ps,
        (void*)root_sig);
    } else {
      pipeline_state_ = PipelineStateBuilder()
                          .SetRootSignature(root_sig)
                          .SetVertexShader(vs)
                          .SetPixelShader(ps)
                          .SetRenderTargetFormat(DXGI_FORMAT_R11G11B10_FLOAT)
                          .SetName(L"BloomDownsample_PSO")
                          .Build(device);
      if (!pipeline_state_) {
        Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[BloomDownsample] PSO creation failed");
      }
    }
  }

  const char* GetName() const override {
    return name_;
  }

  void Execute(const RenderFrameContext& frame, const FramePacket&) override {
    if (!pipeline_state_) return;

    RenderCommandList cmd(frame.command_list, frame.dynamic_allocator, frame.frame_cb, frame.object_cb_allocator);

    frame.command_list->SetPipelineState(pipeline_state_.Get());
    frame.command_list->SetGraphicsRootSignature(shader_manager_->GetRootSignature(Graphics::RSPreset::Standard));

    cmd.BindGlobalSRVTable(frame.global_heap_manager);
    cmd.BindSamplerTable(frame.global_heap_manager);

    auto [src_w, src_h] = frame.render_graph->GetTextureSize(source_handle_);

    BloomDownCB cb_data{
      .source_srv_index = frame.render_graph->GetSrvIndex(source_handle_),
      .texel_size_x = 1.0f / static_cast<float>(src_w),
      .texel_size_y = 1.0f / static_cast<float>(src_h),
      .threshold = config_ ? config_->threshold : 0.0f,
    };

    constexpr auto POST_PROCESS_CB = RootSlot::ConstantBuffer::Light;
    cmd.SetConstantBufferOverride(POST_PROCESS_CB, cb_data);

    frame.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    frame.command_list->DrawInstanced(3, 1, 0, 0);
  }

 private:
  ShaderManager* shader_manager_;
  ComPtr<ID3D12PipelineState> pipeline_state_;
  RenderGraphHandle source_handle_;
  const BloomConfig* config_;
  const char* name_;
};

class BloomUpsamplePass : public IRenderPass {
 public:
  BloomUpsamplePass(
    ID3D12Device* device, ShaderManager* shader_manager, const PassSetup& pass_setup, RenderGraphHandle source_handle, const char* name)
      : shader_manager_(shader_manager), source_handle_(source_handle), name_(name) {
    setup_ = pass_setup;

    auto* vs = shader_manager->GetVertexShader<Graphics::PostProcessBloomUpsampleShader>();
    auto* ps = shader_manager->GetPixelShader<Graphics::PostProcessBloomUpsampleShader>();
    auto* root_sig = shader_manager->GetRootSignature(Graphics::RSPreset::Standard);

    if (!vs || !ps || !root_sig) {
      Logger::LogFormat(LogLevel::Error,
        LogCategory::Graphic,
        Logger::Here(),
        "[BloomUpsample] Shader load failed: vs={} ps={} rs={}",
        (void*)vs,
        (void*)ps,
        (void*)root_sig);
    } else {
      pipeline_state_ = PipelineStateBuilder()
                          .SetRootSignature(root_sig)
                          .SetVertexShader(vs)
                          .SetPixelShader(ps)
                          .SetRenderTargetFormat(DXGI_FORMAT_R11G11B10_FLOAT)
                          .SetBlendMode(BlendMode::Additive)
                          .SetName(L"BloomUpsample_PSO")
                          .Build(device);
      if (!pipeline_state_) {
        Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[BloomUpsample] PSO creation failed");
      }
    }
  }

  const char* GetName() const override {
    return name_;
  }

  void Execute(const RenderFrameContext& frame, const FramePacket&) override {
    if (!pipeline_state_) return;

    RenderCommandList cmd(frame.command_list, frame.dynamic_allocator, frame.frame_cb, frame.object_cb_allocator);

    frame.command_list->SetPipelineState(pipeline_state_.Get());
    frame.command_list->SetGraphicsRootSignature(shader_manager_->GetRootSignature(Graphics::RSPreset::Standard));

    cmd.BindGlobalSRVTable(frame.global_heap_manager);
    cmd.BindSamplerTable(frame.global_heap_manager);

    auto [src_w, src_h] = frame.render_graph->GetTextureSize(source_handle_);

    BloomUpCB cb_data{
      .source_srv_index = frame.render_graph->GetSrvIndex(source_handle_),
      .texel_size_x = 1.0f / static_cast<float>(src_w),
      .texel_size_y = 1.0f / static_cast<float>(src_h),
    };

    constexpr auto POST_PROCESS_CB = RootSlot::ConstantBuffer::Light;
    cmd.SetConstantBufferOverride(POST_PROCESS_CB, cb_data);

    frame.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    frame.command_list->DrawInstanced(3, 1, 0, 0);
  }

 private:
  ShaderManager* shader_manager_;
  ComPtr<ID3D12PipelineState> pipeline_state_;
  RenderGraphHandle source_handle_;
  const char* name_;
};

}  // namespace

void BloomPassGroup::Build(RenderGraph& graph, RenderGraphHandle scene_hdr, const BloomBuildProps& props) {
  uint32_t mip_count = (std::min)(props.config->mip_levels, MAX_MIP_LEVELS);
  if (mip_count == 0) return;

  std::vector<RenderGraphHandle> mip_handles(mip_count);

  for (uint32_t i = 0; i < mip_count; ++i) {
    float scale = 1.0f / static_cast<float>(1u << (i + 1));
    uint32_t mip_width = (std::max)(static_cast<uint32_t>(props.screen_width * scale), 1u);
    uint32_t mip_height = (std::max)(static_cast<uint32_t>(props.screen_height * scale), 1u);

    static constexpr const char* MIP_NAMES[] = {
      "bloom_mip_0",
      "bloom_mip_1",
      "bloom_mip_2",
      "bloom_mip_3",
      "bloom_mip_4",
      "bloom_mip_5",
      "bloom_mip_6",
      "bloom_mip_7",
    };

    mip_handles[i] = graph.CreateRenderTexture({
      .name = MIP_NAMES[i],
      .format = DXGI_FORMAT_R11G11B10_FLOAT,
      .width = mip_width,
      .height = mip_height,
      .device = props.device,
      .scale_factor = scale,
    });
  }

  static constexpr const char* DOWN_NAMES[] = {
    "Bloom Down 0",
    "Bloom Down 1",
    "Bloom Down 2",
    "Bloom Down 3",
    "Bloom Down 4",
    "Bloom Down 5",
    "Bloom Down 6",
    "Bloom Down 7",
  };
  static constexpr const char* UP_NAMES[] = {
    "Bloom Up 0",
    "Bloom Up 1",
    "Bloom Up 2",
    "Bloom Up 3",
    "Bloom Up 4",
    "Bloom Up 5",
    "Bloom Up 6",
    "Bloom Up 7",
  };

  for (uint32_t i = 0; i < mip_count; ++i) {
    RenderGraphHandle source = (i == 0) ? scene_hdr : mip_handles[i - 1];
    const BloomConfig* threshold_config = (i == 0) ? props.config : nullptr;

    PassSetup setup;
    setup.resource_writes = {mip_handles[i]};
    setup.resource_reads = {source};

    graph.AddPass(
      std::make_unique<BloomDownsamplePass>(props.device, props.shader_manager, setup, source, threshold_config, DOWN_NAMES[i]));
  }

  for (uint32_t i = 0; i < mip_count - 1; ++i) {
    uint32_t src_mip = mip_count - 1 - i;
    uint32_t dst_mip = src_mip - 1;

    PassSetup setup;
    setup.resource_writes = {mip_handles[dst_mip]};
    setup.resource_reads = {mip_handles[src_mip]};

    graph.AddPass(std::make_unique<BloomUpsamplePass>(props.device, props.shader_manager, setup, mip_handles[src_mip], UP_NAMES[i]));
  }

  bloom_output_ = mip_handles[0];
}
