#include "depth_view_pass.h"

#include "Framework/Logging/logger.h"
#include "Pipeline/shader_manager.h"
#include "Render/render_graph.h"
#include "d3dx12.h"

DepthViewPass::DepthViewPass(const DepthViewPassProps& props)
    : device_(props.device), shader_manager_(props.shader_manager), depth_handle_(props.depth_handle), config_(props.config) {
  setup_ = props.pass_setup;
  if (!CreatePipelineObjects()) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[DepthViewPass] Failed to create pipeline objects");
  }
}

bool DepthViewPass::CreatePipelineObjects() {
  auto* vs = shader_manager_->GetVertexShader<Graphics::PostProcessDepthViewShader>();
  auto* ps = shader_manager_->GetPixelShader<Graphics::PostProcessDepthViewShader>();
  if (!vs || !ps) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[DepthViewPass] Shaders not loaded");
    return false;
  }

  auto* root_signature = shader_manager_->GetRootSignature(Graphics::RSPreset::Standard);
  if (!root_signature) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[DepthViewPass] Root signature not found");
    return false;
  }

  D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
  pso_desc.pRootSignature = root_signature;
  pso_desc.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
  pso_desc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());
  pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
  pso_desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
  pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
  pso_desc.DepthStencilState.DepthEnable = FALSE;
  pso_desc.DepthStencilState.StencilEnable = FALSE;
  pso_desc.SampleMask = UINT_MAX;
  pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  pso_desc.NumRenderTargets = 1;
  pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
  pso_desc.SampleDesc.Count = 1;

  HRESULT hr = device_->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pipeline_state_));
  if (FAILED(hr)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[DepthViewPass] Failed to create PSO");
    return false;
  }

  pipeline_state_->SetName(L"DepthViewPass_PSO");
  return true;
}

void DepthViewPass::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  (void)packet;

  if (!config_->enabled) return;

  frame.command_list->SetPipelineState(pipeline_state_.Get());

  auto* root_signature = shader_manager_->GetRootSignature(Graphics::RSPreset::Standard);
  frame.command_list->SetGraphicsRootSignature(root_signature);

  struct DepthViewCB {
    uint32_t depth_srv_index;
    float near_plane;
    float far_plane;
    uint32_t padding = 0;
  } cb_data = {
    .depth_srv_index = frame.render_graph->GetSrvIndex(depth_handle_), .near_plane = config_->near_plane, .far_plane = config_->far_plane};

  auto cb_alloc = frame.object_cb_allocator->Allocate<DepthViewCB>();
  memcpy(cb_alloc.cpu_ptr, &cb_data, sizeof(DepthViewCB));
  frame.command_list->SetGraphicsRootConstantBufferView(2, cb_alloc.gpu_ptr);

  frame.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  frame.command_list->DrawInstanced(3, 1, 0, 0);
}
