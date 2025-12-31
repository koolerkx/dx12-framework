#include "material_renderer.h"

#include <DirectXMath.h>

#include "Command/render_command_list.h"
#include "Frame/constant_buffers.h"

using namespace DirectX;

void MaterialRenderer::Record(const RenderFrameContext& frame,
  const std::vector<DrawCommand>& commands,
  const CameraData& camera,
  uint32_t screen_width,
  uint32_t screen_height) {
  if (commands.empty()) {
    return;
  }

  // Find first valid material to initialize the root signature
  const Material* first_material = nullptr;
  for (const auto& draw_cmd : commands) {
    if (draw_cmd.material && draw_cmd.material->IsValid()) {
      first_material = draw_cmd.material;
      break;
    }
  }

  if (!first_material) {
    return;
  }

  RenderCommandList cmd(frame.command_list, frame.dynamic_allocator, frame.frame_cb, frame.object_cb_allocator);

  // 1. Bind descriptor heaps (must be first)
  cmd.BindDescriptorHeaps(frame.global_heap_manager);
  cmd.GetNative()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  // 2. Set initial material to establish the root signature
  cmd.SetMaterial(first_material);

  // 3. Bind global SRV table (requires root signature)
  cmd.BindGlobalSRVTable(frame.global_heap_manager);

  // Setup camera matrices
  XMMATRIX view = XMLoadFloat4x4(&camera.view);
  XMMATRIX proj = XMLoadFloat4x4(&camera.proj);
  XMMATRIX view_proj = XMLoadFloat4x4(&camera.view_proj);

  // 4. Set frame constants (requires root signature)
  FrameCB frame_cb_data = {};
  XMStoreFloat4x4(&frame_cb_data.view, XMMatrixTranspose(view));
  XMStoreFloat4x4(&frame_cb_data.proj, XMMatrixTranspose(proj));
  XMStoreFloat4x4(&frame_cb_data.viewProj, XMMatrixTranspose(view_proj));
  frame_cb_data.cameraPos = camera.position;
  frame_cb_data.screenSize = XMFLOAT2(static_cast<float>(screen_width), static_cast<float>(screen_height));
  cmd.SetFrameConstants(frame_cb_data);

  // Track current material to minimize PSO switches
  const Material* current_material = first_material;

  // Render commands with material batching
  for (const auto& draw_cmd : commands) {
    if (!draw_cmd.mesh || !draw_cmd.material || !draw_cmd.material->IsValid()) {
      continue;
    }

    // Only switch material if it changed (PSO switch optimization)
    if (current_material != draw_cmd.material) {
      cmd.SetMaterial(draw_cmd.material);
      current_material = draw_cmd.material;
    }

    // Set per-object constants
    ObjectCB obj_data = {};
    XMMATRIX world = XMLoadFloat4x4(&draw_cmd.world_matrix);
    XMMATRIX wvp = world * view_proj;
    XMStoreFloat4x4(&obj_data.world, XMMatrixTranspose(world));
    XMStoreFloat4x4(&obj_data.worldViewProj, XMMatrixTranspose(wvp));
    obj_data.color = draw_cmd.color;
    cmd.SetObjectConstants(obj_data);

    // Set material instance data (texture indices, etc)
    cmd.SetMaterialData(draw_cmd.material_instance);

    // Draw mesh
    cmd.DrawMesh(draw_cmd.mesh);
  }
}
