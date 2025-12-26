#include "ui_renderer.h"

#include <DirectXMath.h>

#include "Texture/texture.h"
#include "buffer.h"
#include "constant_buffers.h"
#include "descriptor_heap_manager.h"
#include "mesh.h"

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
    packet.tex = sprite.tex;
    packet.color = sprite.color;
    out.push_back(packet);
  }
}

void UiRenderer::Record(const RenderFrameContext& frame,
  const std::vector<UiDrawPacket>& packets,
  ConstantBuffer<FrameCB>* frame_cb,
  ConstantBuffer<ObjectCB>* obj_cb,
  uint32_t screen_width,
  uint32_t screen_height) {
  using namespace DirectX;

  auto cmd = frame.command_list;

  // Bind pipeline
  cmd->SetGraphicsRootSignature(root_signature_);
  cmd->SetPipelineState(pipeline_state_);
  cmd->SetGraphicsRootDescriptorTable(3, frame.descriptor_manager->GetSrvAllocator().GetHeap()->GetGPUDescriptorHandleForHeapStart());
  cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  // Setup frame CB
  FrameCB frameCBData = {};
  XMMATRIX view = XMMatrixIdentity();
  XMMATRIX proj =
    XMMatrixOrthographicOffCenterLH(0.0f, static_cast<float>(screen_width), static_cast<float>(screen_height), 0.0f, 0.0f, 1.0f);
  XMMATRIX viewProj = view * proj;
  XMStoreFloat4x4(&frameCBData.view, XMMatrixTranspose(view));
  XMStoreFloat4x4(&frameCBData.proj, XMMatrixTranspose(proj));
  XMStoreFloat4x4(&frameCBData.viewProj, XMMatrixTranspose(viewProj));
  frameCBData.cameraPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
  frameCBData.screenSize = XMFLOAT2(static_cast<float>(screen_width), static_cast<float>(screen_height));
  frame_cb->Update(frameCBData);
  cmd->SetGraphicsRootConstantBufferView(0, frame_cb->GetGPUAddress());

  // Draw each sprite
  for (const auto& packet : packets) {
    ObjectCB objData = {};
    XMMATRIX scale = XMMatrixScaling(packet.size.x, packet.size.y, 1.0f);
    XMMATRIX translation = XMMatrixTranslation(packet.pos.x, packet.pos.y, 0.0f);
    XMMATRIX world = scale * translation;
    XMMATRIX wvp = world * viewProj;
    XMStoreFloat4x4(&objData.world, XMMatrixTranspose(world));
    XMStoreFloat4x4(&objData.worldViewProj, XMMatrixTranspose(wvp));
    objData.color = packet.color;
    obj_cb->Update(objData);

    cmd->SetGraphicsRootConstantBufferView(1, obj_cb->GetGPUAddress());

    if (packet.tex) {
      uint32_t texIndex = packet.tex->GetBindlessIndex();
      cmd->SetGraphicsRoot32BitConstants(2, 1, &texIndex, 0);
    }

    quad_mesh_->Draw(cmd);
  }
}
