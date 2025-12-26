#include "ui_renderer.h"

#include <DirectXMath.h>

#include "Texture/texture.h"
#include "constant_buffers.h"
#include "mesh.h"
#include "render_command_list.h"

UiRenderer::UiRenderer(ID3D12RootSignature* root_sig, ID3D12PipelineState* pso, Mesh* quad_mesh)
    : root_signature_(root_sig), pipeline_state_(pso), quad_mesh_(quad_mesh) {
}

void UiRenderer::Build(const RenderWorld& world, std::vector<UiDrawPacket>& out) {
  out.clear();
  out.reserve(world.ui_sprites.size());

  for (const auto& sprite : world.ui_sprites) {
    UiDrawPacket packet;
    packet.pos = sprite.pos;
    packet.size = sprite.size;
    packet.texture = sprite.tex;
    packet.color = sprite.color;
    out.push_back(packet);
  }
}

void UiRenderer::Record(
  const RenderFrameContext& frame, const std::vector<UiDrawPacket>& packets, uint32_t screen_width, uint32_t screen_height) {
  using namespace DirectX;
  RenderCommandList cmd(frame.command_list, frame.dynamic_allocator, frame.frame_cb, frame.object_cb_allocator);

  cmd.SetPipelineState(root_signature_, pipeline_state_);
  cmd.BindHeap(frame.global_heap_manager);
  cmd.GetNative()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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

  for (const auto& packet : packets) {
    ObjectCB objData = {};

    XMMATRIX scale = XMMatrixScaling(packet.size.x, packet.size.y, 1.0f);
    XMMATRIX translation = XMMatrixTranslation(packet.pos.x, packet.pos.y, 0.0f);
    XMMATRIX world = scale * translation;
    XMMATRIX wvp = world * viewProj;

    XMStoreFloat4x4(&objData.world, XMMatrixTranspose(world));
    XMStoreFloat4x4(&objData.worldViewProj, XMMatrixTranspose(wvp));
    objData.color = packet.color;

    cmd.SetObjectConstants(objData);

    if (packet.texture) {
      cmd.BindTextureIndex(2, packet.texture->GetBindlessIndex());
    }
    cmd.DrawMesh(quad_mesh_);
  }
}
