#include "depth_normal_pass.h"

#include "Command/render_command_list.h"
#include "Frame/constant_buffers.h"
#include "Resource/Mesh/mesh_buffer_pool.h"
#include "Resource/Mesh/mesh_descriptor.h"
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

  if (!pipeline_state_) return false;

  auto* instanced_vs = shader_manager_->GetVertexShader<Graphics::DepthNormalInstancedShader>();
  if (instanced_vs) {
    instanced_pipeline_state_ = PipelineStateBuilder()
                                  .SetRootSignature(root_signature)
                                  .SetVertexShader(instanced_vs)
                                  .SetPixelShader(ps)
                                  .SetInputLayout(Graphics::DepthNormalInstancedShader::GetInputLayout())
                                  .SetRenderTargetFormat(DXGI_FORMAT_R16G16B16A16_FLOAT)
                                  .SetDepthStencilFormat(DXGI_FORMAT_D32_FLOAT)
                                  .EnableDepthTest()
                                  .SetCullMode(D3D12_CULL_MODE_BACK)
                                  .SetName(L"DepthNormalPass_Instanced_PSO")
                                  .Build(device_);
  }

  return true;
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

  MeshBufferPool* mesh_pool = frame.mesh_buffer_pool;
  bool unified_buffers_bound = false;

  for (const auto& draw_cmd : packet.commands) {
    if (!draw_cmd.depth_write) continue;
    if (!draw_cmd.mesh && !draw_cmd.UsesBindlessMesh()) continue;
    // CPU instances (sprites) skip prepass — only structured (3D model) instances participate
    if (draw_cmd.IsInstanced()) continue;

    if (draw_cmd.UsesBindlessMesh()) {
      if (!unified_buffers_bound) {
        auto vbv = mesh_pool->GetVertexBufferView();
        auto ibv = mesh_pool->GetIndexBufferView();
        frame.command_list->IASetVertexBuffers(0, 1, &vbv);
        frame.command_list->IASetIndexBuffer(&ibv);
        unified_buffers_bound = true;
      }
      const MeshDescriptor* desc = mesh_pool->GetDescriptor(draw_cmd.mesh_handle);
      if (!desc) continue;

      if (draw_cmd.IsStructuredInstanced()) {
        if (!instanced_pipeline_state_) continue;
        frame.command_list->SetPipelineState(instanced_pipeline_state_.Get());
        cmd.SetInstanceBufferSRV(draw_cmd.instance_buffer_address);
        frame.command_list->DrawIndexedInstanced(
          desc->index_count, draw_cmd.instance_count, desc->index_offset, desc->vertex_offset, 0);
        frame.command_list->SetPipelineState(pipeline_state_.Get());
      } else {
        ObjectCB obj_data = {};
        obj_data.world = draw_cmd.world_matrix;
        obj_data.normalMatrix = draw_cmd.world_matrix.Inverted().Transposed();
        cmd.SetObjectConstants(obj_data);
        frame.command_list->DrawIndexedInstanced(
          desc->index_count, 1, desc->index_offset, desc->vertex_offset, 0);
      }
      continue;
    }

    unified_buffers_bound = false;

    if (draw_cmd.IsStructuredInstanced()) {
      if (!instanced_pipeline_state_) continue;

      frame.command_list->SetPipelineState(instanced_pipeline_state_.Get());
      cmd.SetInstanceBufferSRV(draw_cmd.instance_buffer_address);

      const Mesh* mesh = draw_cmd.mesh;
      auto vbv = mesh->GetVertexBuffer().GetView();
      auto ibv = mesh->GetIndexBuffer().GetView();
      frame.command_list->IASetVertexBuffers(0, 1, &vbv);
      frame.command_list->IASetIndexBuffer(&ibv);

      for (size_t i = 0; i < mesh->GetSubMeshCount(); ++i) {
        const auto& sub = mesh->GetSubMesh(i);
        frame.command_list->DrawIndexedInstanced(
          sub.indexCount, draw_cmd.instance_count, sub.startIndexLocation, sub.baseVertexLocation, 0);
      }

      frame.command_list->SetPipelineState(pipeline_state_.Get());
      continue;
    }

    ObjectCB obj_data = {};
    obj_data.world = draw_cmd.world_matrix;
    obj_data.normalMatrix = draw_cmd.world_matrix.Inverted().Transposed();
    cmd.SetObjectConstants(obj_data);

    cmd.DrawMesh(draw_cmd.mesh);
  }
}
