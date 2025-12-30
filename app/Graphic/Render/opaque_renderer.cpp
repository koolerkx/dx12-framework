#include "opaque_renderer.h"

#include <DirectXMath.h>

#include <algorithm>

#include "Command/render_command_list.h"
#include "Frame/constant_buffers.h"
#include "Resource/Texture/texture.h"

OpaqueRenderer::OpaqueRenderer(ID3D12RootSignature* root_sig, ID3D12PipelineState* pso) : root_signature_(root_sig), pipeline_state_(pso) {
}

void OpaqueRenderer::Build(const FramePacket& packet, std::vector<OpaqueDrawCommand>& out_cache) {
  out_cache = packet.opaque_pass;

  // Sort front-to-back for better depth testing performance
  std::sort(out_cache.begin(), out_cache.end(), [](const OpaqueDrawCommand& a, const OpaqueDrawCommand& b) { return a.depth < b.depth; });
}

void OpaqueRenderer::Record(const RenderFrameContext& frame,
  const std::vector<OpaqueDrawCommand>& commands,
  const CameraData& camera,
  uint32_t screen_width,
  uint32_t screen_height) {
  using namespace DirectX;

  if (commands.empty()) {
    return;
  }

  RenderCommandList cmd(frame.command_list, frame.dynamic_allocator, frame.frame_cb, frame.object_cb_allocator);

  cmd.SetPipelineState(root_signature_, pipeline_state_);
  cmd.BindHeap(frame.global_heap_manager);
  cmd.GetNative()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  XMMATRIX view = XMLoadFloat4x4(&camera.view);
  XMMATRIX proj = XMLoadFloat4x4(&camera.proj);
  XMMATRIX viewProj = camera.GetViewProjMatrix();

  FrameCB frameCBData = {};
  XMStoreFloat4x4(&frameCBData.view, XMMatrixTranspose(view));
  XMStoreFloat4x4(&frameCBData.proj, XMMatrixTranspose(proj));
  XMStoreFloat4x4(&frameCBData.viewProj, XMMatrixTranspose(viewProj));
  frameCBData.cameraPos = camera.position;
  frameCBData.screenSize = XMFLOAT2(static_cast<float>(screen_width), static_cast<float>(screen_height));

  cmd.SetFrameConstants(frameCBData);

  for (const auto& command : commands) {
    if (!command.mesh) {
      continue;
    }

    ObjectCB objData = {};
    XMMATRIX world = XMLoadFloat4x4(&command.world_matrix);
    XMMATRIX wvp = world * viewProj;

    XMStoreFloat4x4(&objData.world, XMMatrixTranspose(world));
    XMStoreFloat4x4(&objData.worldViewProj, XMMatrixTranspose(wvp));
    objData.color = command.color;

    cmd.SetObjectConstants(objData);

    if (command.texture) {
      cmd.BindTextureIndex(2, command.texture->GetBindlessIndex());
    }

    cmd.DrawMesh(command.mesh);
  }
}
