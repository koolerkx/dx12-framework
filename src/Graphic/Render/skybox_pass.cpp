#include "skybox_pass.h"

#include "Command/render_command_list.h"
#include "Frame/constant_buffers.h"
#include "Framework/Logging/logger.h"
#include "Pipeline/pipeline_state_builder.h"
#include "Pipeline/root_parameter_slots.h"
#include "Pipeline/shader_descriptors.h"
#include "Pipeline/shader_manager.h"
#include "Pipeline/vertex_types.h"

SkyboxPass::SkyboxPass(ID3D12Device* device, ShaderManager* shader_manager, PassSetup pass_setup)
    : device_(device), shader_manager_(shader_manager) {
  setup_ = std::move(pass_setup);
  if (!CreatePipelineObjects()) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[SkyboxPass] Failed to create pipeline objects");
  }
  if (!CreateCubeMesh()) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[SkyboxPass] Failed to create cube mesh");
  }
}

bool SkyboxPass::CreatePipelineObjects() {
  auto* vs = shader_manager_->GetVertexShader<Graphics::SkyboxShader>();
  auto* ps = shader_manager_->GetPixelShader<Graphics::SkyboxShader>();
  if (!vs || !ps) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[SkyboxPass] Shaders not loaded");
    return false;
  }

  auto* root_signature = shader_manager_->GetRootSignature(Graphics::RSPreset::Standard);
  if (!root_signature) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[SkyboxPass] Root signature not found");
    return false;
  }

  pipeline_state_ = PipelineStateBuilder()
                      .SetRootSignature(root_signature)
                      .SetVertexShader(vs)
                      .SetPixelShader(ps)
                      .SetInputLayout(Graphics::SkyboxShader::GetInputLayout())
                      .SetDepthTest(true)
                      .SetDepthWrite(false)
                      .SetDepthComparisonFunc(D3D12_COMPARISON_FUNC_LESS_EQUAL)
                      .SetRenderTargetFormat(DXGI_FORMAT_R16G16B16A16_FLOAT)
                      .SetDepthStencilFormat(DXGI_FORMAT_D32_FLOAT)
                      .SetName(L"SkyboxPass_PSO")
                      .Build(device_);

  return pipeline_state_ != nullptr;
}

bool SkyboxPass::CreateCubeMesh() {
  using namespace Graphics::Vertex;

  // Unit cube vertices (±1.0)
  SkyboxVertex vertices[] = {
    {{-1.0f, -1.0f, -1.0f}},
    {{-1.0f, 1.0f, -1.0f}},
    {{1.0f, 1.0f, -1.0f}},
    {{1.0f, -1.0f, -1.0f}},
    {{-1.0f, -1.0f, 1.0f}},
    {{-1.0f, 1.0f, 1.0f}},
    {{1.0f, 1.0f, 1.0f}},
    {{1.0f, -1.0f, 1.0f}},
  };

  // 12 triangles, 36 indices (winding for viewing from inside)
  uint16_t indices[] = {
    // Front face (-Z)
    0,
    2,
    1,
    0,
    3,
    2,
    // Back face (+Z)
    4,
    5,
    6,
    4,
    6,
    7,
    // Left face (-X)
    4,
    1,
    5,
    4,
    0,
    1,
    // Right face (+X)
    3,
    6,
    2,
    3,
    7,
    6,
    // Top face (+Y)
    1,
    6,
    5,
    1,
    2,
    6,
    // Bottom face (-Y)
    4,
    3,
    0,
    4,
    7,
    3,
  };

  return cube_mesh_.Create(device_, vertices, 8, indices, 36);
}

void SkyboxPass::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  const auto& bg = packet.background;

  if (bg.mode != BackgroundMode::ClearColor && bg.cubemap_srv_index == UINT32_MAX) return;
  if (!pipeline_state_) return;

  RenderCommandList cmd(frame.command_list, frame.dynamic_allocator, frame.frame_cb, frame.object_cb_allocator);

  // TODO: replace with cmd.SetMaterial() when skybox becomes a proper Material
  frame.command_list->SetPipelineState(pipeline_state_.Get());
  auto* root_signature = shader_manager_->GetRootSignature(Graphics::RSPreset::Standard);
  frame.command_list->SetGraphicsRootSignature(root_signature);

  cmd.BindGlobalSRVTable(frame.global_heap_manager);
  cmd.BindSamplerTable(frame.global_heap_manager);

  FrameCB frame_cb_data = {};
  frame_cb_data.view = packet.main_camera.view;
  frame_cb_data.proj = packet.main_camera.proj;
  frame_cb_data.viewProj = packet.main_camera.view_proj;
  frame_cb_data.invView = packet.main_camera.inv_view;
  frame_cb_data.invProj = packet.main_camera.inv_proj;
  frame_cb_data.cameraPos = packet.main_camera.position;
  frame_cb_data.screenSize = Math::Vector2(static_cast<float>(frame.screen_width), static_cast<float>(frame.screen_height));
  cmd.SetFrameConstants(frame_cb_data);

  uint32_t srv_index = (bg.mode == BackgroundMode::ClearColor) ? UINT32_MAX : bg.cubemap_srv_index;

  struct SkyboxCB {
    uint32_t cubemap_srv_index;
    uint32_t padding[3];
  };
  static_assert(sizeof(SkyboxCB) == 16);

  SkyboxCB skybox_cb = {srv_index, {}};
  constexpr auto SKYBOX_CB = RootSlot::ConstantBuffer::Light;
  cmd.SetConstantBufferOverride(SKYBOX_CB, skybox_cb);

  if (bg.mode == BackgroundMode::ClearColor) {
    ObjectCB object_cb = {};
    object_cb.color = bg.clear_color;
    cmd.SetObjectConstants(object_cb);
  }

  frame.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  cube_mesh_.Draw(frame.command_list);
}
