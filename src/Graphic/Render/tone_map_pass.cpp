#include "tone_map_pass.h"

#include <array>

#include "Framework/Logging/logger.h"
#include "Pipeline/material_manager.h"
#include "Pipeline/shader_manager.h"
#include "Presentation/hdr_render_target.h"
#include "Presentation/swapchain_manager.h"
#include "d3dx12.h"

static constexpr std::array<float, 4> CLEAR_COLOR = {0.0f, 0.0f, 0.0f, 1.0f};

ToneMapPass::ToneMapPass(ID3D12Device* device, MaterialManager* material_manager, ShaderManager* shader_manager)
    : device_(device), material_manager_(material_manager), shader_manager_(shader_manager) {
  if (!CreatePipelineObjects()) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[ToneMapPass] Failed to create pipeline objects");
  }
}

bool ToneMapPass::CreatePipelineObjects() {
  // Get shaders
  auto* vs = shader_manager_->GetVertexShader(Graphics::ShaderID::PostProcessToneMap);
  auto* ps = shader_manager_->GetPixelShader(Graphics::ShaderID::PostProcessToneMap);
  if (!vs || !ps) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[ToneMapPass] Shaders not loaded");
    return false;
  }

  // Get root signature
  auto* root_signature = shader_manager_->GetRootSignature(Graphics::RSPreset::Standard);
  if (!root_signature) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[ToneMapPass] Root signature not found");
    return false;
  }

  // Create PSO
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
  pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;  // UNORM swapchain (sRGB in shader)
  pso_desc.SampleDesc.Count = 1;

  HRESULT hr = device_->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pipeline_state_));
  if (FAILED(hr)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[ToneMapPass] Failed to create PSO");
    return false;
  }

  pipeline_state_->SetName(L"ToneMapPass_PSO");
  return true;
}

void ToneMapPass::PreExecute(const RenderFrameContext& frame, const FramePacket& packet) {
  (void)packet;

  // Transition HDR RT from RENDER_TARGET to PIXEL_SHADER_RESOURCE
  hdr_rt_->TransitionTo(frame.command_list, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

  // Transition swapchain from PRESENT to RENDER_TARGET
  swapchain_manager_->TransitionToRenderTarget(frame.command_list);

  // Clear swapchain to black
  D3D12_CPU_DESCRIPTOR_HANDLE swapchain_rtv = swapchain_manager_->GetCurrentRTV();
  frame.command_list->ClearRenderTargetView(swapchain_rtv, CLEAR_COLOR.data(), 0, nullptr);

  // Set swapchain as render target (no depth for post-processing)
  frame.command_list->OMSetRenderTargets(1, &swapchain_rtv, FALSE, nullptr);
}

void ToneMapPass::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  (void)packet;

  // Set pipeline state
  frame.command_list->SetPipelineState(pipeline_state_.Get());

  // Set root signature
  auto* root_signature = shader_manager_->GetRootSignature(Graphics::RSPreset::Standard);
  frame.command_list->SetGraphicsRootSignature(root_signature);

  // Upload ToneMapCB (exposure, debug mode, HDR RT SRV index)
  struct ToneMapCB {
    float exposure;
    uint32_t debug_mode;
    uint32_t hdr_srv_index;
    uint32_t padding = 0;
  } cb_data = {.exposure = config_->exposure, .debug_mode = debug_->debug_view ? 1u : 0u, .hdr_srv_index = hdr_rt_->GetSrvIndex()};

  // Allocate and upload constant buffer
  auto cb_alloc = frame.object_cb_allocator->Allocate<ToneMapCB>();
  memcpy(cb_alloc.cpu_ptr, &cb_data, sizeof(ToneMapCB));
  frame.command_list->SetGraphicsRootConstantBufferView(2, cb_alloc.gpu_ptr);

  // Set descriptor heaps (already set by BeginFrame)
  // HDR RT SRV is in the static heap at index hdr_rt_->GetSrvIndex()

  // Draw full-screen triangle
  frame.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  frame.command_list->DrawInstanced(3, 1, 0, 0);
}
