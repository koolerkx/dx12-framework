#include "smaa_pass_group.h"

#include "Command/render_command_list.h"
#include "Core/types.h"
#include "Framework/Logging/logger.h"
#include "Pipeline/pipeline_state_builder.h"
#include "Pipeline/shader_descriptors.h"
#include "Pipeline/shader_manager.h"
#include "Render/render_graph.h"
#include "Resource/Texture/texture_manager.h"
#include "ThirdParty/SMAA/AreaTex.h"
#include "ThirdParty/SMAA/SearchTex.h"

namespace {

struct SMAAEdgeCB {
  DirectX::XMFLOAT4 rt_metrics;
  uint32_t color_srv_index;
  uint32_t _pad[3] = {0, 0, 0};
};
static_assert(sizeof(SMAAEdgeCB) == 32);

struct SMAABlendWeightCB {
  DirectX::XMFLOAT4 rt_metrics;
  uint32_t edges_srv_index;
  uint32_t area_srv_index;
  uint32_t search_srv_index;
  uint32_t _pad = 0;
};
static_assert(sizeof(SMAABlendWeightCB) == 32);

struct SMAANeighborhoodCB {
  DirectX::XMFLOAT4 rt_metrics;
  uint32_t color_srv_index;
  uint32_t blend_srv_index;
  uint32_t _pad[2] = {0, 0};
};
static_assert(sizeof(SMAANeighborhoodCB) == 32);

DirectX::XMFLOAT4 ComputeRTMetrics(const RenderGraph* graph, RenderGraphHandle handle) {
  auto [w, h] = graph->GetTextureSize(handle);
  return {1.0f / static_cast<float>(w), 1.0f / static_cast<float>(h), static_cast<float>(w), static_cast<float>(h)};
}

std::vector<uint8_t> PadRGToRGBA(const unsigned char* src, uint32_t width, uint32_t height) {
  std::vector<uint8_t> rgba(width * height * 4);
  for (uint32_t i = 0; i < width * height; ++i) {
    rgba[i * 4 + 0] = src[i * 2 + 0];
    rgba[i * 4 + 1] = src[i * 2 + 1];
    rgba[i * 4 + 2] = 0;
    rgba[i * 4 + 3] = 255;
  }
  return rgba;
}

std::vector<uint8_t> PadRToRGBA(const unsigned char* src, uint32_t width, uint32_t height) {
  std::vector<uint8_t> rgba(width * height * 4);
  for (uint32_t i = 0; i < width * height; ++i) {
    rgba[i * 4 + 0] = src[i];
    rgba[i * 4 + 1] = 0;
    rgba[i * 4 + 2] = 0;
    rgba[i * 4 + 3] = 255;
  }
  return rgba;
}

class SMAAEdgePass : public IRenderPass {
 public:
  SMAAEdgePass(ID3D12Device* device,
    ShaderManager* shader_manager,
    const PassSetup& pass_setup,
    RenderGraphHandle color_handle,
    RenderGraphHandle output_handle)
      : shader_manager_(shader_manager), color_handle_(color_handle), output_handle_(output_handle) {
    setup_ = pass_setup;

    auto* vs = shader_manager->GetVertexShader<Graphics::PostProcessSMAAEdgeShader>();
    auto* ps = shader_manager->GetPixelShader<Graphics::PostProcessSMAAEdgeShader>();
    auto* root_sig = shader_manager->GetRootSignature(Graphics::RSPreset::Standard);

    if (!vs || !ps || !root_sig) {
      Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[SMAAEdgePass] Shader load failed");
    } else {
      pipeline_state_ = PipelineStateBuilder()
                          .SetRootSignature(root_sig)
                          .SetVertexShader(vs)
                          .SetPixelShader(ps)
                          .SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM)
                          .SetName(L"SMAAEdge_PSO")
                          .Build(device);
    }
  }

  const char* GetName() const override {
    return "SMAA Edge Detection";
  }

  void Execute(const RenderFrameContext& frame, const FramePacket&) override {
    if (!pipeline_state_) return;

    RenderCommandList cmd(frame.command_list, frame.dynamic_allocator, frame.object_cb_allocator);

    frame.command_list->SetPipelineState(pipeline_state_.Get());
    frame.command_list->SetGraphicsRootSignature(shader_manager_->GetRootSignature(Graphics::RSPreset::Standard));

    cmd.BindGlobalSRVTable(frame.global_heap_manager);
    cmd.BindSamplerTable(frame.global_heap_manager);

    SMAAEdgeCB cb_data{
      .rt_metrics = ComputeRTMetrics(frame.render_graph, output_handle_),
      .color_srv_index = frame.render_graph->GetSrvIndex(color_handle_),
    };

    constexpr auto POST_PROCESS_CB = RootSlot::ConstantBuffer::Light;
    cmd.SetConstantBufferOverride(POST_PROCESS_CB, cb_data);

    frame.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    frame.command_list->DrawInstanced(3, 1, 0, 0);
  }

 private:
  ShaderManager* shader_manager_;
  ComPtr<ID3D12PipelineState> pipeline_state_;
  RenderGraphHandle color_handle_;
  RenderGraphHandle output_handle_;
};

class SMAABlendWeightPass : public IRenderPass {
 public:
  SMAABlendWeightPass(ID3D12Device* device,
    ShaderManager* shader_manager,
    const PassSetup& pass_setup,
    RenderGraphHandle edges_handle,
    RenderGraphHandle output_handle,
    uint32_t area_srv_index,
    uint32_t search_srv_index)
      : shader_manager_(shader_manager),
        edges_handle_(edges_handle),
        output_handle_(output_handle),
        area_srv_index_(area_srv_index),
        search_srv_index_(search_srv_index) {
    setup_ = pass_setup;

    auto* vs = shader_manager->GetVertexShader<Graphics::PostProcessSMAABlendWeightShader>();
    auto* ps = shader_manager->GetPixelShader<Graphics::PostProcessSMAABlendWeightShader>();
    auto* root_sig = shader_manager->GetRootSignature(Graphics::RSPreset::Standard);

    if (!vs || !ps || !root_sig) {
      Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[SMAABlendWeightPass] Shader load failed");
    } else {
      pipeline_state_ = PipelineStateBuilder()
                          .SetRootSignature(root_sig)
                          .SetVertexShader(vs)
                          .SetPixelShader(ps)
                          .SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM)
                          .SetName(L"SMAABlendWeight_PSO")
                          .Build(device);
    }
  }

  const char* GetName() const override {
    return "SMAA Blend Weight";
  }

  void Execute(const RenderFrameContext& frame, const FramePacket&) override {
    if (!pipeline_state_) return;

    RenderCommandList cmd(frame.command_list, frame.dynamic_allocator, frame.object_cb_allocator);

    frame.command_list->SetPipelineState(pipeline_state_.Get());
    frame.command_list->SetGraphicsRootSignature(shader_manager_->GetRootSignature(Graphics::RSPreset::Standard));

    cmd.BindGlobalSRVTable(frame.global_heap_manager);
    cmd.BindSamplerTable(frame.global_heap_manager);

    SMAABlendWeightCB cb_data{
      .rt_metrics = ComputeRTMetrics(frame.render_graph, output_handle_),
      .edges_srv_index = frame.render_graph->GetSrvIndex(edges_handle_),
      .area_srv_index = area_srv_index_,
      .search_srv_index = search_srv_index_,
    };

    constexpr auto POST_PROCESS_CB = RootSlot::ConstantBuffer::Light;
    cmd.SetConstantBufferOverride(POST_PROCESS_CB, cb_data);

    frame.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    frame.command_list->DrawInstanced(3, 1, 0, 0);
  }

 private:
  ShaderManager* shader_manager_;
  ComPtr<ID3D12PipelineState> pipeline_state_;
  RenderGraphHandle edges_handle_;
  RenderGraphHandle output_handle_;
  uint32_t area_srv_index_;
  uint32_t search_srv_index_;
};

class SMAANeighborhoodPass : public IRenderPass {
 public:
  SMAANeighborhoodPass(ID3D12Device* device,
    ShaderManager* shader_manager,
    const PassSetup& pass_setup,
    RenderGraphHandle color_handle,
    RenderGraphHandle blend_handle,
    RenderGraphHandle output_handle)
      : shader_manager_(shader_manager), color_handle_(color_handle), blend_handle_(blend_handle), output_handle_(output_handle) {
    setup_ = pass_setup;

    auto* vs = shader_manager->GetVertexShader<Graphics::PostProcessSMAANeighborhoodShader>();
    auto* ps = shader_manager->GetPixelShader<Graphics::PostProcessSMAANeighborhoodShader>();
    auto* root_sig = shader_manager->GetRootSignature(Graphics::RSPreset::Standard);

    if (!vs || !ps || !root_sig) {
      Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[SMAANeighborhoodPass] Shader load failed");
    } else {
      pipeline_state_ = PipelineStateBuilder()
                          .SetRootSignature(root_sig)
                          .SetVertexShader(vs)
                          .SetPixelShader(ps)
                          .SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM)
                          .SetName(L"SMAANeighborhood_PSO")
                          .Build(device);
    }
  }

  const char* GetName() const override {
    return "SMAA Neighborhood Blending";
  }

  void Execute(const RenderFrameContext& frame, const FramePacket&) override {
    if (!pipeline_state_) return;

    RenderCommandList cmd(frame.command_list, frame.dynamic_allocator, frame.object_cb_allocator);

    frame.command_list->SetPipelineState(pipeline_state_.Get());
    frame.command_list->SetGraphicsRootSignature(shader_manager_->GetRootSignature(Graphics::RSPreset::Standard));

    cmd.BindGlobalSRVTable(frame.global_heap_manager);
    cmd.BindSamplerTable(frame.global_heap_manager);

    SMAANeighborhoodCB cb_data{
      .rt_metrics = ComputeRTMetrics(frame.render_graph, output_handle_),
      .color_srv_index = frame.render_graph->GetSrvIndex(color_handle_),
      .blend_srv_index = frame.render_graph->GetSrvIndex(blend_handle_),
    };

    constexpr auto POST_PROCESS_CB = RootSlot::ConstantBuffer::Light;
    cmd.SetConstantBufferOverride(POST_PROCESS_CB, cb_data);

    frame.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    frame.command_list->DrawInstanced(3, 1, 0, 0);
  }

 private:
  ShaderManager* shader_manager_;
  ComPtr<ID3D12PipelineState> pipeline_state_;
  RenderGraphHandle color_handle_;
  RenderGraphHandle blend_handle_;
  RenderGraphHandle output_handle_;
};

}  // namespace

void SMAAPassGroup::Build(RenderGraph& graph, RenderGraphHandle ldr_input, const SMAAPipelineBuildProps& props) {
  // SMAA lookup textures from https://github.com/iryoku/smaa/tree/master/Textures
  auto area_rgba = PadRGToRGBA(areaTexBytes, AREATEX_WIDTH, AREATEX_HEIGHT);
  auto area_tex = props.texture_manager->LoadTextureFromRawPixels("smaa_area_tex", area_rgba.data(), AREATEX_WIDTH, AREATEX_HEIGHT, false);

  auto search_rgba = PadRToRGBA(searchTexBytes, SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT);
  auto search_tex =
    props.texture_manager->LoadTextureFromRawPixels("smaa_search_tex", search_rgba.data(), SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, false);

  if (!area_tex || !search_tex) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[SMAAPassGroup] Failed to load lookup textures");
    return;
  }

  uint32_t area_srv = area_tex->GetBindlessIndex();
  uint32_t search_srv = search_tex->GetBindlessIndex();

  auto edges_rt = graph.CreateRenderTexture({
    .name = "smaa_edges_rt",
    .format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .width = props.context.width,
    .height = props.context.height,
    .device = props.context.device,
    .clear_color = colors::Black,
  });

  auto blend_rt = graph.CreateRenderTexture({
    .name = "smaa_blend_rt",
    .format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .width = props.context.width,
    .height = props.context.height,
    .device = props.context.device,
    .clear_color = colors::Black,
  });

  smaa_output_ = graph.CreateRenderTexture({
    .name = "smaa_output_rt",
    .format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .width = props.context.width,
    .height = props.context.height,
    .device = props.context.device,
  });

  PassSetup edge_setup;
  edge_setup.resource_writes = {edges_rt};
  edge_setup.resource_reads = {ldr_input};

  AddPass(graph, std::make_unique<SMAAEdgePass>(props.context.device, props.context.shader_manager, edge_setup, ldr_input, edges_rt));

  PassSetup blend_weight_setup;
  blend_weight_setup.resource_writes = {blend_rt};
  blend_weight_setup.resource_reads = {edges_rt};

  AddPass(graph,
    std::make_unique<SMAABlendWeightPass>(
      props.context.device, props.context.shader_manager, blend_weight_setup, edges_rt, blend_rt, area_srv, search_srv));

  PassSetup neighborhood_setup;
  neighborhood_setup.resource_writes = {smaa_output_};
  neighborhood_setup.resource_reads = {ldr_input, blend_rt};

  AddPass(graph,
    std::make_unique<SMAANeighborhoodPass>(
      props.context.device, props.context.shader_manager, neighborhood_setup, ldr_input, blend_rt, smaa_output_));
}
