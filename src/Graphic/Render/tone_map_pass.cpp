#include "tone_map_pass.h"

#include "Framework/Logging/logger.h"
#include "Pipeline/material_manager.h"
#include "Pipeline/pipeline_state_builder.h"
#include "Pipeline/shader_manager.h"
#include "Render/render_graph.h"

ToneMapPass::ToneMapPass(const ToneMapPassProps& props)
    : device_(props.device),
      material_manager_(props.material_manager),
      shader_manager_(props.shader_manager),
      hdr_handle_(props.hdr_handle),
      debug_(props.debug) {
  setup_ = props.pass_setup;
  if (!CreatePipelineObjects()) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[ToneMapPass] Failed to create pipeline objects");
  }
}

bool ToneMapPass::CreatePipelineObjects() {
  auto* vs = shader_manager_->GetVertexShader<Graphics::PostProcessToneMapShader>();
  auto* ps = shader_manager_->GetPixelShader<Graphics::PostProcessToneMapShader>();
  if (!vs || !ps) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[ToneMapPass] Shaders not loaded");
    return false;
  }

  auto* root_signature = shader_manager_->GetRootSignature(Graphics::RSPreset::Standard);
  if (!root_signature) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[ToneMapPass] Root signature not found");
    return false;
  }

  pipeline_state_ = PipelineStateBuilder()
                      .SetRootSignature(root_signature)
                      .SetVertexShader(vs)
                      .SetPixelShader(ps)
                      .SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM)
                      .SetName(L"ToneMapPass_PSO")
                      .Build(device_);

  return pipeline_state_ != nullptr;
}

void ToneMapPass::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  (void)packet;

  frame.command_list->SetPipelineState(pipeline_state_.Get());

  auto* root_signature = shader_manager_->GetRootSignature(Graphics::RSPreset::Standard);
  frame.command_list->SetGraphicsRootSignature(root_signature);

  struct ToneMapCB {
    float exposure;
    uint32_t debug_mode;
    uint32_t hdr_srv_index;
    uint32_t padding = 0;
  } cb_data = {.exposure = packet.main_camera.exposure,
    .debug_mode = debug_->debug_view ? 1u : 0u,
    .hdr_srv_index = frame.render_graph->GetSrvIndex(hdr_handle_)};

  auto cb_alloc = frame.object_cb_allocator->Allocate<ToneMapCB>();
  memcpy(cb_alloc.cpu_ptr, &cb_data, sizeof(ToneMapCB));
  frame.command_list->SetGraphicsRootConstantBufferView(2, cb_alloc.gpu_ptr);

  frame.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  frame.command_list->DrawInstanced(3, 1, 0, 0);
}
