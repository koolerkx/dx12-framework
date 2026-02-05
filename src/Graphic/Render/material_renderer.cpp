#include "material_renderer.h"

#include <DirectXMath.h>

#include <bit>

#include "Command/render_command_list.h"
#include "Frame/constant_buffers.h"
#include "Pipeline/material.h"

using namespace DirectX;

namespace SortKey {

uint64_t MaterialFirst(const DrawCommand& cmd, bool front_to_back) {
  if (!cmd.material) {
    return UINT64_MAX;
  }

  uint32_t rs_key = cmd.material->GetRootSignatureKey() & 0xFFFF;
  uint32_t pso_key = cmd.material->GetPSOKey() & 0xFFFF;
  uint64_t compressed_material = (static_cast<uint64_t>(rs_key) << 48) | (static_cast<uint64_t>(pso_key) << 32);

  uint32_t depth_key = std::bit_cast<uint32_t>(cmd.depth);
  if (!front_to_back) {
    depth_key = ~depth_key;
  }

  return compressed_material | depth_key;
}

uint64_t DepthFirst(const DrawCommand& cmd, bool front_to_back) {
  if (!cmd.material) {
    return UINT64_MAX;
  }

  uint32_t depth_key = std::bit_cast<uint32_t>(cmd.depth);
  if (!front_to_back) {
    depth_key = ~depth_key;
  }

  uint32_t rs_key = cmd.material->GetRootSignatureKey() & 0xFFFF;
  uint32_t pso_key = cmd.material->GetPSOKey() & 0xFFFF;
  uint64_t material_key = (static_cast<uint64_t>(rs_key) << 16) | static_cast<uint64_t>(pso_key);

  return (static_cast<uint64_t>(depth_key) << 32) | material_key;
}

}  // namespace SortKey

void MaterialRenderer::Record(const RenderFrameContext& frame,
  const std::vector<DrawCommand>& commands,
  const CameraData& camera,
  uint32_t screen_width,
  uint32_t screen_height) {
  if (commands.empty()) {
    return;
  }

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

  cmd.BindDescriptorHeaps(frame.global_heap_manager);
  cmd.GetNative()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  cmd.SetMaterial(first_material);
  cmd.BindGlobalSRVTable(frame.global_heap_manager);
  cmd.BindSamplerTable(frame.global_heap_manager);

  XMMATRIX view = XMLoadFloat4x4(&camera.view);
  XMMATRIX proj = XMLoadFloat4x4(&camera.proj);
  XMMATRIX view_proj = XMLoadFloat4x4(&camera.view_proj);

  FrameCB frame_cb_data = {};
  XMStoreFloat4x4(&frame_cb_data.view, view);
  XMStoreFloat4x4(&frame_cb_data.proj, proj);
  XMStoreFloat4x4(&frame_cb_data.viewProj, view_proj);
  frame_cb_data.cameraPos = camera.position;
  frame_cb_data.screenSize = XMFLOAT2(static_cast<float>(screen_width), static_cast<float>(screen_height));
  cmd.SetFrameConstants(frame_cb_data);

  const Material* current_material = first_material;

  for (const auto& draw_cmd : commands) {
    if (!draw_cmd.material || !draw_cmd.material->IsValid()) {
      continue;
    }
    if (!draw_cmd.mesh) {
      continue;
    }

    if (current_material != draw_cmd.material) {
      cmd.SetMaterial(draw_cmd.material);
      current_material = draw_cmd.material;
    }

    if (draw_cmd.IsInstanced()) {
      RecordInstanced(cmd, draw_cmd);
    } else {
      RecordSingle(cmd, draw_cmd, view_proj);
    }
  }
}

void MaterialRenderer::RecordSingle(RenderCommandList& cmd, const DrawCommand& draw_cmd, const XMMATRIX& view_proj) {
  cmd.SetMaterialData(draw_cmd.material_instance);

  ObjectCB obj_data = {};
  XMMATRIX world = XMLoadFloat4x4(&draw_cmd.world_matrix);
  XMMATRIX wvp = world * view_proj;
  XMStoreFloat4x4(&obj_data.world, world);
  XMStoreFloat4x4(&obj_data.worldViewProj, wvp);
  obj_data.color = draw_cmd.color;
  obj_data.uvOffset = draw_cmd.uv_offset;
  obj_data.uvScale = draw_cmd.uv_scale;
  obj_data.samplerIndex = draw_cmd.material_instance.sampler_index;
  cmd.SetObjectConstants(obj_data);

  cmd.DrawMesh(draw_cmd.mesh);
}

void MaterialRenderer::RecordInstanced(RenderCommandList& cmd, const DrawCommand& draw_cmd) {
  cmd.SetMaterialData(draw_cmd.material_instance);

  // Instanced rendering gets per-instance data from vertex buffer
  ObjectCB obj_data = {};
  obj_data.samplerIndex = draw_cmd.material_instance.sampler_index;
  cmd.SetObjectConstants(obj_data);

  cmd.DrawMeshInstanced(draw_cmd.mesh, draw_cmd.instances);
}
