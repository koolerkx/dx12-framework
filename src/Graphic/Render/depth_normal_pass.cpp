#include "depth_normal_pass.h"

#include "Command/render_command_list.h"
#include "Frame/constant_buffers.h"
#include "Framework/Logging/logger.h"
#include "Pipeline/pipeline_state_builder.h"
#include "Pipeline/shader_descriptors.h"
#include "Pipeline/shader_manager.h"
#include "Render/draw_command_resolver.h"
#include "Render/prepass_record_utils.h"
#include "Render/resolved_command_grouper.h"

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

  resolved_commands_.clear();
  DrawCommandResolver::ResolveContext resolve_ctx{
    .material_manager = frame.material_manager,
    .mesh_buffer_pool = frame.mesh_buffer_pool,
    .instance_allocator = frame.object_cb_allocator,
    .shadow_enabled = false,
  };

  std::vector<RenderRequest> filtered_single;
  for (const auto& req : packet.single_requests) {
    if (req.render_settings.depth_write) filtered_single.push_back(req);
  }
  DrawCommandResolver::ResolveSingleRequests(resolve_ctx, filtered_single, resolved_commands_);

  std::vector<InternalInstancedRequest> filtered_instanced;
  for (const auto& req : packet.instanced_requests) {
    if (req.request.render_settings.depth_write) filtered_instanced.push_back(req);
  }
  DrawCommandResolver::ResolveInstancedRequests(resolve_ctx, filtered_instanced, packet.instance_data_pool, resolved_commands_);

  ResolvedCommandGrouper::GroupForPrepass(resolved_commands_, resolve_ctx.instance_allocator);

  RecordPrepassCommands(cmd, resolved_commands_, [](const ResolvedDrawCommand& dc) {
    ObjectCB obj{};
    obj.world = dc.world_matrix;
    obj.normalMatrix = dc.world_matrix.Inverted().Transposed();
    obj.flags = dc.object_flags;
    return obj;
  });
}
