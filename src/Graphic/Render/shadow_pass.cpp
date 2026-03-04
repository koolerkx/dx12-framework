#include "shadow_pass.h"

#include <algorithm>
#include <array>
#include <cstdio>

#include "Command/render_command_list.h"
#include "Frame/constant_buffers.h"
#include "Framework/Logging/logger.h"
#include "Framework/Math/Math.h"
#include "Pipeline/pipeline_state_builder.h"
#include "Pipeline/shader_descriptors.h"
#include "Pipeline/shader_manager.h"
#include "Resource/Mesh/mesh_buffer_pool.h"
#include "Resource/Mesh/mesh_descriptor.h"
#include "shadow_config.h"

using Math::Matrix4;
using Math::Vector3;

namespace {

// C_i = λ * n*(f/n)^(i/m) + (1-λ) * (n + i/m*(f-n))
void ComputeCascadeSplits(float camera_near, float shadow_distance, uint32_t count, float out[ShadowCascadeConfig::MAX_CASCADES]) {
  float n = Math::Max(camera_near, 0.01f);
  float f = shadow_distance;
  float m = static_cast<float>(count);
  constexpr float lambda = ShadowCascadeConfig::SPLIT_LAMBDA;

  for (uint32_t i = 0; i < count; ++i) {
    float t = static_cast<float>(i + 1) / m;
    float log_split = n * std::pow(f / n, t);
    float lin_split = n + t * (f - n);
    out[i] = lambda * log_split + (1.0f - lambda) * lin_split;
  }
}

}  // namespace

ShadowPass::ShadowPass(const Props& props)
    : device_(props.device), shader_manager_(props.shader_manager), shadow_data_(props.shadow_data), cascade_index_(props.cascade_index) {
  setup_ = props.pass_setup;
  std::snprintf(name_buffer_, sizeof(name_buffer_), "Shadow Pass %u", cascade_index_);
  if (!CreatePipelineState()) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[ShadowPass] Failed to create pipeline state");
  }
}

bool ShadowPass::CreatePipelineState() {
  auto* vs = shader_manager_->GetVertexShader<Graphics::ShadowDepthShader>();
  if (!vs) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[ShadowPass] Shadow depth VS not loaded");
    return false;
  }

  auto* root_signature = shader_manager_->GetRootSignature(Graphics::RSPreset::Standard);
  if (!root_signature) {
    return false;
  }

  pipeline_state_ =
    PipelineStateBuilder()
      .SetRootSignature(root_signature)
      .SetVertexShader(vs)
      .SetInputLayout(Graphics::ShadowDepthShader::GetInputLayout())
      .SetCullMode(D3D12_CULL_MODE_BACK)
      .EnableDepthTest()
      .SetDepthBias(ShadowHardwareConfig::DEPTH_BIAS, ShadowHardwareConfig::DEPTH_BIAS_CLAMP, ShadowHardwareConfig::SLOPE_SCALED_DEPTH_BIAS)
      .SetDepthStencilFormat(DXGI_FORMAT_D32_FLOAT)
      .SetName(L"ShadowPass_PSO")
      .Build(device_);

  if (!pipeline_state_) return false;

  auto* instanced_vs = shader_manager_->GetVertexShader<Graphics::ShadowDepthInstancedShader>();
  if (instanced_vs) {
    instanced_pipeline_state_ =
      PipelineStateBuilder()
        .SetRootSignature(root_signature)
        .SetVertexShader(instanced_vs)
        .SetInputLayout(Graphics::ShadowDepthInstancedShader::GetInputLayout())
        .SetCullMode(D3D12_CULL_MODE_BACK)
        .EnableDepthTest()
        .SetDepthBias(
          ShadowHardwareConfig::DEPTH_BIAS, ShadowHardwareConfig::DEPTH_BIAS_CLAMP, ShadowHardwareConfig::SLOPE_SCALED_DEPTH_BIAS)
        .SetDepthStencilFormat(DXGI_FORMAT_D32_FLOAT)
        .SetName(L"ShadowPass_Instanced_PSO")
        .Build(device_);
  }

  return true;
}

Matrix4 ShadowPass::ComputeLightViewProj(
  const CameraData& camera, const Vector3& light_dir, float near_dist, float far_dist, float light_distance) const {
  Matrix4 inv_view_proj = (camera.view * camera.proj).Inverted();

  std::array<Vector3, 8> ndc_corners = {{
    {-1, -1, 0},
    {1, -1, 0},
    {-1, 1, 0},
    {1, 1, 0},
    {-1, -1, 1},
    {1, -1, 1},
    {-1, 1, 1},
    {1, 1, 1},
  }};

  std::array<Vector3, 8> world_corners;
  for (size_t i = 0; i < 8; ++i) {
    world_corners[i] = inv_view_proj.TransformPoint(ndc_corners[i]);
  }

  float camera_near = std::abs(camera.inv_proj.TransformPoint(Vector3(0, 0, 0)).z);
  float camera_far = std::abs(camera.inv_proj.TransformPoint(Vector3(0, 0, 1)).z);
  float depth_range = camera_far - camera_near;
  if (depth_range > 0.0f) {
    float t_near = std::clamp((near_dist - camera_near) / depth_range, 0.0f, 1.0f);
    float t_far = std::clamp((far_dist - camera_near) / depth_range, 0.0f, 1.0f);
    std::array<Vector3, 8> clipped;
    for (size_t i = 0; i < 4; ++i) {
      Vector3 ray = world_corners[i + 4] - world_corners[i];
      clipped[i] = world_corners[i] + ray * t_near;
      clipped[i + 4] = world_corners[i] + ray * t_far;
    }
    world_corners = clipped;
  }

  Vector3 center = Vector3::Zero;
  for (const auto& corner : world_corners) {
    center += corner;
  }
  center /= 8.0f;

  Vector3 normalized_light = light_dir;
  normalized_light.Normalize();

  Vector3 light_up = Vector3::Up;
  if (std::abs(normalized_light.Dot(light_up)) > 0.99f) {
    light_up = Vector3::Right;
  }

  Vector3 light_pos = center - normalized_light * light_distance;
  Matrix4 light_view = Matrix4::CreateLookAt(light_pos, center, light_up);

  float min_x = 1e30f;
  float max_x = -1e30f;
  float min_y = 1e30f;
  float max_y = -1e30f;
  float min_z = 1e30f;
  float max_z = -1e30f;

  for (const auto& corner : world_corners) {
    Vector3 light_space = light_view.TransformPoint(corner);
    min_x = Math::Min(min_x, light_space.x);
    max_x = Math::Max(max_x, light_space.x);
    min_y = Math::Min(min_y, light_space.y);
    max_y = Math::Max(max_y, light_space.y);
    min_z = Math::Min(min_z, light_space.z);
    max_z = Math::Max(max_z, light_space.z);
  }

  float padding = 10.0f;
  min_x -= padding;
  max_x += padding;
  min_y -= padding;
  max_y += padding;
  min_z -= padding;
  max_z += padding;

  float resolution = static_cast<float>(shadow_data_->shadow_map_resolution);
  float texel_x = (max_x - min_x) / resolution;
  float texel_y = (max_y - min_y) / resolution;
  min_x = std::floor(min_x / texel_x) * texel_x;
  max_x = std::floor(max_x / texel_x) * texel_x;
  min_y = std::floor(min_y / texel_y) * texel_y;
  max_y = std::floor(max_y / texel_y) * texel_y;

  Matrix4 light_proj = Matrix4::CreateOrthographicOffCenter(min_x, max_x, min_y, max_y, min_z, max_z);

  return light_view * light_proj;
}

void ShadowPass::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  if (!pipeline_state_ || !shadow_data_) return;
  if (!packet.shadow.enabled) return;
  uint32_t allocated_count = shadow_data_->cascade_count;
  uint32_t effective_count = (std::min)(packet.shadow.cascade_count, allocated_count);
  if (cascade_index_ >= effective_count) return;

  if (cascade_index_ == 0) {
    float camera_near = std::abs(packet.main_camera.inv_proj.TransformPoint(Vector3(0, 0, 0)).z);
    ComputeCascadeSplits(camera_near, packet.shadow.shadow_distance, effective_count, shadow_data_->cascade_split_distances);
    shadow_data_->cascade_count = effective_count;
  }

  float near_dist = (cascade_index_ == 0) ? 0.0f : shadow_data_->cascade_split_distances[cascade_index_ - 1];
  float far_dist = shadow_data_->cascade_split_distances[cascade_index_];

  Matrix4 light_view_proj =
    ComputeLightViewProj(packet.main_camera, packet.lighting.direction, near_dist, far_dist, packet.shadow.light_distance);
  shadow_data_->light_view_proj[cascade_index_] = light_view_proj;

  RenderCommandList cmd(frame.command_list, frame.dynamic_allocator, frame.frame_cb, frame.object_cb_allocator);

  auto* root_signature = shader_manager_->GetRootSignature(Graphics::RSPreset::Standard);
  frame.command_list->SetGraphicsRootSignature(root_signature);
  frame.command_list->SetPipelineState(pipeline_state_.Get());
  frame.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  FrameCB frame_cb_data = {};
  frame_cb_data.viewProj = light_view_proj;
  cmd.SetFrameConstants(frame_cb_data);

  MeshBufferPool* mesh_pool = frame.mesh_buffer_pool;
  bool unified_buffers_bound = false;

  for (const auto& draw_cmd : packet.commands) {
    if (!HasTag(draw_cmd.tags, RenderTag::CastShadow)) continue;
    if (!draw_cmd.mesh && !draw_cmd.UsesBindlessMesh()) continue;
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
        frame.command_list->DrawIndexedInstanced(desc->index_count, draw_cmd.instance_count, desc->index_offset, desc->vertex_offset, 0);
        frame.command_list->SetPipelineState(pipeline_state_.Get());
      } else {
        ObjectCB obj_data = {};
        obj_data.world = draw_cmd.world_matrix;
        cmd.SetObjectConstants(obj_data);
        frame.command_list->DrawIndexedInstanced(desc->index_count, 1, desc->index_offset, desc->vertex_offset, 0);
      }
      continue;
    }

    unified_buffers_bound = false;

    // DEPRECATED(Phase4): Remove after bindless migration complete
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
    cmd.SetObjectConstants(obj_data);

    // DEPRECATED(Phase4): Remove after bindless migration complete
    draw_cmd.mesh->Draw(frame.command_list);
  }
}
