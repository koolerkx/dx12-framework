#include "normal_view_pass.h"

#include "Command/render_command_list.h"
#include "Descriptor/descriptor_heap_manager.h"
#include "Framework/Logging/logger.h"
#include "Pipeline/pipeline_state_builder.h"
#include "Pipeline/shader_manager.h"
#include "Render/render_graph.h"

NormalViewPass::NormalViewPass(const NormalViewPassProps& props)
    : device_(props.device), shader_manager_(props.shader_manager), source_handle_(props.source_handle) {
  setup_ = props.pass_setup;
  if (!CreatePipelineObjects()) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[NormalViewPass] Failed to create pipeline objects");
  }
}

bool NormalViewPass::CreatePipelineObjects() {
  auto* vs = shader_manager_->GetVertexShader<Graphics::PostProcessNormalViewShader>();
  auto* ps = shader_manager_->GetPixelShader<Graphics::PostProcessNormalViewShader>();
  if (!vs || !ps) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[NormalViewPass] Shaders not loaded");
    return false;
  }

  auto* root_signature = shader_manager_->GetRootSignature(Graphics::RSPreset::Standard);
  if (!root_signature) return false;

  pipeline_state_ = PipelineStateBuilder()
                      .SetRootSignature(root_signature)
                      .SetVertexShader(vs)
                      .SetPixelShader(ps)
                      .SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM)
                      .SetName(L"NormalViewPass_PSO")
                      .Build(device_);

  return pipeline_state_ != nullptr;
}

void NormalViewPass::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  (void)packet;

  RenderCommandList cmd(frame.command_list, frame.dynamic_allocator, frame.frame_cb, frame.object_cb_allocator);

  auto* root_signature = shader_manager_->GetRootSignature(Graphics::RSPreset::Standard);
  frame.command_list->SetGraphicsRootSignature(root_signature);
  frame.command_list->SetPipelineState(pipeline_state_.Get());

  cmd.BindGlobalSRVTable(frame.global_heap_manager);
  cmd.BindSamplerTable(frame.global_heap_manager);

  struct NormalViewCB {
    uint32_t src_srv_index;
    uint32_t padding[3] = {};
  } cb_data = {.src_srv_index = frame.render_graph->GetSrvIndex(source_handle_)};

  auto cb_alloc = frame.object_cb_allocator->Allocate<NormalViewCB>();
  memcpy(cb_alloc.cpu_ptr, &cb_data, sizeof(NormalViewCB));
  frame.command_list->SetGraphicsRootConstantBufferView(2, cb_alloc.gpu_ptr);

  frame.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  frame.command_list->DrawInstanced(3, 1, 0, 0);
}
