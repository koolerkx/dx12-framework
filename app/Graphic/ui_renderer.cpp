#include "ui_renderer.h"

#include <DirectXMath.h>

#include <algorithm>  // for std::sort

#include "Texture/texture.h"
#include "constant_buffers.h"
#include "mesh.h"
#include "render_command_list.h"

UiRenderer::UiRenderer(ID3D12RootSignature* root_sig, ID3D12PipelineState* pso, Mesh* quad_mesh)
    : root_signature_(root_sig), pipeline_state_(pso), quad_mesh_(quad_mesh) {
}

void UiRenderer::Build(const FramePacket& packet, std::vector<UiDrawCommand>& out_cache) {
  out_cache = packet.ui_pass;

  // 画家のアルゴリズム (Painter's algorithm)
  std::sort(out_cache.begin(), out_cache.end(), [](const UiDrawCommand& a, const UiDrawCommand& b) { return a.layer_id < b.layer_id; });
}

void UiRenderer::Record(
  const RenderFrameContext& frame, const std::vector<UiDrawCommand>& commands, uint32_t screen_width, uint32_t screen_height) {
  using namespace DirectX;
  RenderCommandList cmd(frame.command_list, frame.dynamic_allocator, frame.frame_cb, frame.object_cb_allocator);

  cmd.SetPipelineState(root_signature_, pipeline_state_);
  cmd.BindHeap(frame.global_heap_manager);
  cmd.GetNative()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  // Setup UI Projection (Orthographic)
  XMMATRIX view = XMMatrixIdentity();
  XMMATRIX proj =
    XMMatrixOrthographicOffCenterLH(0.0f, static_cast<float>(screen_width), static_cast<float>(screen_height), 0.0f, 0.0f, 1.0f);
  XMMATRIX viewProj = view * proj;

  // Setup Frame CB
  FrameCB frameCBData = {};
  XMStoreFloat4x4(&frameCBData.view, XMMatrixTranspose(view));
  XMStoreFloat4x4(&frameCBData.proj, XMMatrixTranspose(proj));
  XMStoreFloat4x4(&frameCBData.viewProj, XMMatrixTranspose(viewProj));
  frameCBData.screenSize = XMFLOAT2(static_cast<float>(screen_width), static_cast<float>(screen_height));

  cmd.SetFrameConstants(frameCBData);

  for (const auto& command : commands) {
    ObjectCB objData = {};

    // Matrix Calculation:
    // 1. Apply Size Scaling (The sprite size)
    // 2. Apply Transform Matrix (Position, Rotation, and Parent Scale from GameObject)
    XMMATRIX sizeScale = XMMatrixScaling(command.size.x, command.size.y, 1.0f);
    XMMATRIX transformMatrix = XMLoadFloat4x4(&command.world_matrix);

    // Final World = Size * Transform
    XMMATRIX world = sizeScale * transformMatrix;
    XMMATRIX wvp = world * viewProj;

    XMStoreFloat4x4(&objData.world, XMMatrixTranspose(world));
    XMStoreFloat4x4(&objData.worldViewProj, XMMatrixTranspose(wvp));
    objData.color = command.color;

    cmd.SetObjectConstants(objData);

    if (command.texture) {
      cmd.BindTextureIndex(2, command.texture->GetBindlessIndex());
    }
    cmd.DrawMesh(quad_mesh_);
  }
}
