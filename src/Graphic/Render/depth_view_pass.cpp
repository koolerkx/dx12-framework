#include "depth_view_pass.h"

#include "Command/render_command_list.h"
#include "Framework/Logging/logger.h"
#include "Pipeline/pipeline_state_builder.h"
#include "Pipeline/shader_manager.h"
#include "Render/render_graph.h"

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

  pipeline_state_ = PipelineStateBuilder()
                      .SetRootSignature(root_signature)
                      .SetVertexShader(vs)
                      .SetPixelShader(ps)
                      .SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM)
                      .SetName(L"DepthViewPass_PSO")
                      .Build(device_);

  return pipeline_state_ != nullptr;
}

void DepthViewPass::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  (void)packet;

  if (!config_->enabled) return;

  RenderCommandList cmd(frame.command_list, frame.dynamic_allocator, frame.frame_cb, frame.object_cb_allocator);

  auto* root_signature = shader_manager_->GetRootSignature(Graphics::RSPreset::Standard);
  frame.command_list->SetGraphicsRootSignature(root_signature);
  frame.command_list->SetPipelineState(pipeline_state_.Get());

  cmd.BindGlobalSRVTable(frame.global_heap_manager);
  cmd.BindSamplerTable(frame.global_heap_manager);

  struct DepthViewCB {
    uint32_t depth_srv_index;
    float near_plane;
    float far_plane;
    uint32_t padding = 0;
  } cb_data = {
    .depth_srv_index = frame.render_graph->GetSrvIndex(depth_handle_),
    .near_plane = config_->near_plane,
    .far_plane = config_->far_plane,
  };

  constexpr auto POST_PROCESS_CB = RootSlot::ConstantBuffer::Light;
  cmd.SetConstantBufferOverride(POST_PROCESS_CB, cb_data);

  frame.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  frame.command_list->DrawInstanced(3, 1, 0, 0);
}
