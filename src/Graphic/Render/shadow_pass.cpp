#include "shadow_pass.h"

#include <array>

#include "Command/render_command_list.h"
#include "Frame/constant_buffers.h"
#include "Framework/Logging/logger.h"
#include "Framework/Math/Math.h"
#include "Pipeline/shader_descriptors.h"
#include "Pipeline/shader_manager.h"
#include "d3dx12.h"
#include "shadow_config.h"

using Math::Matrix4;
using Math::Vector3;

ShadowPass::ShadowPass(const Props& props) : device_(props.device), shader_manager_(props.shader_manager), shadow_data_(props.shadow_data) {
  setup_ = props.pass_setup;
  if (!CreatePipelineState()) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[ShadowPass] Failed to create pipeline state");
  }
}

bool ShadowPass::CreatePipelineState() {
  auto* vs = shader_manager_->GetVertexShader<Graphics::ShadowDepthShader>();
  auto* ps = shader_manager_->GetPixelShader<Graphics::ShadowDepthShader>();
  if (!vs) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[ShadowPass] Shadow depth VS not loaded");
    return false;
  }

  auto* root_signature = shader_manager_->GetRootSignature(Graphics::RSPreset::Standard);
  if (!root_signature) {
    return false;
  }

  auto input_layout = Graphics::ShadowDepthShader::GetInputLayout();

  D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
  pso_desc.pRootSignature = root_signature;
  pso_desc.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
  if (ps) {
    pso_desc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());
  }
  pso_desc.InputLayout = {input_layout.data(), static_cast<UINT>(input_layout.size())};
  pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
  pso_desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
  pso_desc.RasterizerState.DepthBias = ShadowHardwareConfig::DEPTH_BIAS;
  pso_desc.RasterizerState.SlopeScaledDepthBias = ShadowHardwareConfig::SLOPE_SCALED_DEPTH_BIAS;
  pso_desc.RasterizerState.DepthBiasClamp = ShadowHardwareConfig::DEPTH_BIAS_CLAMP;
  pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
  pso_desc.DepthStencilState.DepthEnable = TRUE;
  pso_desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
  pso_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
  pso_desc.DepthStencilState.StencilEnable = FALSE;
  pso_desc.SampleMask = UINT_MAX;
  pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  pso_desc.NumRenderTargets = 0;
  pso_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
  pso_desc.SampleDesc.Count = 1;

  HRESULT hr = device_->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pipeline_state_));
  if (FAILED(hr)) {
    Logger::LogFormat(LogLevel::Error,
      LogCategory::Graphic,
      Logger::Here(),
      "[ShadowPass] Failed to create PSO, HRESULT=0x{:08X}",
      static_cast<uint32_t>(hr));
    return false;
  }
  pipeline_state_->SetName(L"ShadowPass_PSO");
  return true;
}

Matrix4 ShadowPass::ComputeLightViewProj(const CameraData& camera, const Vector3& light_dir, float shadow_distance, float light_distance) const {
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

  if (shadow_distance > 0.0f) {
    float camera_far = std::abs(camera.inv_proj.TransformPoint(Vector3(0, 0, 1)).z);
    if (camera_far > 0.0f && shadow_distance < camera_far) {
      float t = shadow_distance / camera_far;
      for (size_t i = 4; i < 8; ++i) {
        world_corners[i] = world_corners[i - 4] + (world_corners[i] - world_corners[i - 4]) * t;
      }
    }
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

  Matrix4 light_view_proj = ComputeLightViewProj(packet.main_camera, packet.lighting.direction, packet.shadow.shadow_distance, packet.shadow.light_distance);
  shadow_data_->light_view_proj = light_view_proj;

  RenderCommandList cmd(frame.command_list, frame.dynamic_allocator, frame.frame_cb, frame.object_cb_allocator);

  auto* root_signature = shader_manager_->GetRootSignature(Graphics::RSPreset::Standard);
  frame.command_list->SetGraphicsRootSignature(root_signature);
  frame.command_list->SetPipelineState(pipeline_state_.Get());
  frame.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  FrameCB frame_cb_data = {};
  frame_cb_data.viewProj = light_view_proj;
  cmd.SetFrameConstants(frame_cb_data);

  for (const auto& draw_cmd : packet.commands) {
    if (!HasTag(draw_cmd.tags, RenderTag::CastShadow)) continue;
    if (!draw_cmd.mesh) continue;
    if (draw_cmd.IsInstanced()) continue;

    ObjectCB obj_data = {};
    obj_data.world = draw_cmd.world_matrix;
    cmd.SetObjectConstants(obj_data);

    draw_cmd.mesh->Draw(frame.command_list);
  }
}
