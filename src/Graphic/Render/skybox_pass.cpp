#include "skybox_pass.h"

#include "Command/render_command_list.h"
#include "Frame/constant_buffers.h"
#include "Framework/Logging/logger.h"
#include "Pipeline/root_parameter_slots.h"
#include "Pipeline/shader_descriptors.h"
#include "Pipeline/shader_manager.h"
#include "Pipeline/vertex_types.h"
#include "d3dx12.h"

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

  D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
  pso_desc.pRootSignature = root_signature;
  pso_desc.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
  pso_desc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());

  auto input_layout = Graphics::SkyboxShader::GetInputLayout();
  pso_desc.InputLayout = {input_layout.data(), static_cast<UINT>(input_layout.size())};

  pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
  pso_desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
  pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

  pso_desc.DepthStencilState.DepthEnable = TRUE;
  pso_desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
  pso_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
  pso_desc.DepthStencilState.StencilEnable = FALSE;

  pso_desc.SampleMask = UINT_MAX;
  pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  pso_desc.NumRenderTargets = 1;
  pso_desc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
  pso_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
  pso_desc.SampleDesc.Count = 1;

  HRESULT hr = device_->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pipeline_state_));
  if (FAILED(hr)) {
    Logger::LogFormat(LogLevel::Error,
      LogCategory::Graphic,
      Logger::Here(),
      "[SkyboxPass] Failed to create PSO, HRESULT=0x{:08X}",
      static_cast<uint32_t>(hr));
    return false;
  }
  pipeline_state_->SetName(L"SkyboxPass_PSO");
  return true;
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
  auto skybox_alloc = frame.object_cb_allocator->Allocate<SkyboxCB>();
  memcpy(skybox_alloc.cpu_ptr, &skybox_cb, sizeof(SkyboxCB));
  frame.command_list->SetGraphicsRootConstantBufferView(RootSlot::ToIndex(RootSlot::ConstantBuffer::Light), skybox_alloc.gpu_ptr);

  if (bg.mode == BackgroundMode::ClearColor) {
    ObjectCB object_cb = {};
    object_cb.color = bg.clear_color;
    cmd.SetObjectConstants(object_cb);
  }

  frame.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  cube_mesh_.Draw(frame.command_list);
}
