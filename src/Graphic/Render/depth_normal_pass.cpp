#include "depth_normal_pass.h"

#include "Command/render_command_list.h"
#include "Frame/constant_buffers.h"
#include "Framework/Logging/logger.h"
#include "Pipeline/pipeline_state_builder.h"
#include "Pipeline/shader_descriptors.h"
#include "Pipeline/shader_manager.h"

DepthNormalPass::DepthNormalPass(const Props& props) : device_(props.device), shader_manager_(props.shader_manager) {
  setup_ = props.pass_setup;
  if (!CreatePipelineState()) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[DepthNormalPass] Failed to create pipeline state");
  }
}

bool DepthNormalPass::CreatePipelineState() {
  auto* vs = shader_manager_->GetVertexShader<Graphics::DepthNormalShader>();
  auto* ps = shader_manager_->GetPixelShader<Graphics::DepthNormalShader>();
  if (!vs || !ps) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[DepthNormalPass] Shader not loaded");
    return false;
  }

  auto* root_signature = shader_manager_->GetRootSignature(Graphics::RSPreset::Standard);
  if (!root_signature) {
    return false;
  }

  pipeline_state_ = PipelineStateBuilder()
                      .SetRootSignature(root_signature)
                      .SetVertexShader(vs)
                      .SetPixelShader(ps)
                      .SetInputLayout(Graphics::DepthNormalShader::GetInputLayout())
                      .SetRenderTargetFormat(DXGI_FORMAT_R16G16B16A16_FLOAT)
                      .SetDepthStencilFormat(DXGI_FORMAT_D32_FLOAT)
                      .EnableDepthTest()
                      .SetCullMode(D3D12_CULL_MODE_BACK)
                      .SetName(L"DepthNormalPass_PSO")
                      .Build(device_);

  return pipeline_state_ != nullptr;
}

void DepthNormalPass::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  if (!pipeline_state_) return;

  RenderCommandList cmd(frame.command_list, frame.dynamic_allocator, frame.frame_cb, frame.object_cb_allocator);

  auto* root_signature = shader_manager_->GetRootSignature(Graphics::RSPreset::Standard);
  frame.command_list->SetGraphicsRootSignature(root_signature);
  frame.command_list->SetPipelineState(pipeline_state_.Get());
  frame.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  FrameCB frame_cb_data = {};
  frame_cb_data.view = packet.main_camera.view;
  frame_cb_data.proj = packet.main_camera.proj;
  frame_cb_data.viewProj = packet.main_camera.view * packet.main_camera.proj;
  cmd.SetFrameConstants(frame_cb_data);

  for (const auto& draw_cmd : packet.commands) {
    if (!draw_cmd.depth_write) continue;
    if (!draw_cmd.mesh) continue;
    if (draw_cmd.IsInstanced()) continue;

    ObjectCB obj_data = {};
    obj_data.world = draw_cmd.world_matrix;
    obj_data.normalMatrix = draw_cmd.world_matrix.Inverted().Transposed();
    cmd.SetObjectConstants(obj_data);

    cmd.DrawMesh(draw_cmd.mesh);
  }
}
