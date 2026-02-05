#include "material_renderer.h"

#include <DirectXMath.h>

#include <type_traits>

#include "Command/render_command_list.h"
#include "Frame/constant_buffers.h"
#include "Pipeline/material.h"

using namespace DirectX;

// ============================================================================
// NEW: Unified Command Recording
// ============================================================================

void MaterialRenderer::Record(const RenderFrameContext& frame,
  const std::vector<RenderCommand>& commands,
  const CameraData& camera,
  uint32_t screen_width,
  uint32_t screen_height) {
  if (commands.empty()) {
    return;
  }

  // Find first valid material to initialize the root signature
  const Material* first_material = nullptr;
  for (const auto& render_cmd : commands) {
    const Material* mat = render_cmd.command.material;
    if (mat && mat->IsValid()) {
      first_material = mat;
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

  // 3. Bind global descriptor tables (requires root signature)
  cmd.BindGlobalSRVTable(frame.global_heap_manager);
  cmd.BindSamplerTable(frame.global_heap_manager);

  // Setup camera matrices
  XMMATRIX view = XMLoadFloat4x4(&camera.view);
  XMMATRIX proj = XMLoadFloat4x4(&camera.proj);
  XMMATRIX view_proj = XMLoadFloat4x4(&camera.view_proj);

  // 4. Set frame constants (requires root signature)
  FrameCB frame_cb_data = {};
  XMStoreFloat4x4(&frame_cb_data.view, view);
  XMStoreFloat4x4(&frame_cb_data.proj, proj);
  XMStoreFloat4x4(&frame_cb_data.viewProj, view_proj);
  frame_cb_data.cameraPos = camera.position;
  frame_cb_data.screenSize = XMFLOAT2(static_cast<float>(screen_width), static_cast<float>(screen_height));
  cmd.SetFrameConstants(frame_cb_data);

  // Track current material to minimize PSO switches
  const Material* current_material = first_material;

  // Render commands with material batching
  for (const auto& render_cmd : commands) {
    const DrawCommand& draw_cmd = render_cmd.command;

    if (!draw_cmd.material || !draw_cmd.material->IsValid()) {
      continue;
    }
    if (!draw_cmd.mesh) {
      continue;
    }

    // Only switch material if it changed (PSO switch optimization)
    if (current_material != draw_cmd.material) {
      cmd.SetMaterial(draw_cmd.material);
      current_material = draw_cmd.material;
    }

    // Dispatch based on IsInstanced()
    if (draw_cmd.IsInstanced()) {
      RecordInstanced(cmd, draw_cmd);
    } else {
      RecordSingle(cmd, draw_cmd, view_proj);
    }
  }
}

void MaterialRenderer::RecordSingle(RenderCommandList& cmd, const DrawCommand& draw_cmd, const XMMATRIX& view_proj) {
  // Set material instance data (texture indices, etc)
  cmd.SetMaterialData(draw_cmd.material_instance);

  // Set per-object constants for single draw rendering
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

  // Draw mesh (single instance)
  cmd.DrawMesh(draw_cmd.mesh);
}

void MaterialRenderer::RecordInstanced(RenderCommandList& cmd, const DrawCommand& draw_cmd) {
  // Set material instance data (texture indices, etc)
  cmd.SetMaterialData(draw_cmd.material_instance);

  // Set minimal ObjectCB for sampler index (instanced rendering gets per-instance data from vertex buffer)
  ObjectCB obj_data = {};
  obj_data.samplerIndex = draw_cmd.material_instance.sampler_index;
  cmd.SetObjectConstants(obj_data);

  // Draw all instances in a single draw call
  cmd.DrawMeshInstanced(draw_cmd.mesh, draw_cmd.instances);
}

// ============================================================================
// DEPRECATED: Legacy Variant-based Recording
// ============================================================================

void MaterialRenderer::RecordLegacy(const RenderFrameContext& frame,
  const std::vector<DrawCommandVariant>& commands,
  const CameraData& camera,
  uint32_t screen_width,
  uint32_t screen_height) {
  if (commands.empty()) {
    return;
  }

  // Find first valid material to initialize the root signature
  const Material* first_material = nullptr;
  for (const auto& cmd_variant : commands) {
    const Material* mat = std::visit([](auto&& cmd) { return cmd.material; }, cmd_variant);
    if (mat && mat->IsValid()) {
      first_material = mat;
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

  // 3. Bind global descriptor tables (requires root signature)
  cmd.BindGlobalSRVTable(frame.global_heap_manager);
  cmd.BindSamplerTable(frame.global_heap_manager);

  // Setup camera matrices
  XMMATRIX view = XMLoadFloat4x4(&camera.view);
  XMMATRIX proj = XMLoadFloat4x4(&camera.proj);
  XMMATRIX view_proj = XMLoadFloat4x4(&camera.view_proj);

  // 4. Set frame constants (requires root signature)
  FrameCB frame_cb_data = {};
  XMStoreFloat4x4(&frame_cb_data.view, view);
  XMStoreFloat4x4(&frame_cb_data.proj, proj);
  XMStoreFloat4x4(&frame_cb_data.viewProj, view_proj);
  frame_cb_data.cameraPos = camera.position;
  frame_cb_data.screenSize = XMFLOAT2(static_cast<float>(screen_width), static_cast<float>(screen_height));
  cmd.SetFrameConstants(frame_cb_data);

  // Track current material to minimize PSO switches
  const Material* current_material = first_material;

  // Render commands with material batching and type-based dispatch
  for (const auto& cmd_variant : commands) {
    // Extract material from variant
    const Material* material = std::visit([](auto&& cmd) { return cmd.material; }, cmd_variant);
    if (!material || !material->IsValid()) {
      continue;
    }

    // Extract mesh from variant
    const Mesh* mesh = std::visit([](auto&& cmd) { return cmd.mesh; }, cmd_variant);
    if (!mesh) {
      continue;
    }

    // Only switch material if it changed (PSO switch optimization)
    if (current_material != material) {
      cmd.SetMaterial(material);
      current_material = material;
    }

    // Dispatch to appropriate recording method based on command type
    std::visit(
      [&](auto&& draw_cmd) {
        using T = std::decay_t<decltype(draw_cmd)>;
        if constexpr (std::is_same_v<T, SingleDrawCommand>) {
          RecordSingleLegacy(cmd, draw_cmd, view_proj);
        } else if constexpr (std::is_same_v<T, InstanceDrawCommand>) {
          RecordInstanceLegacy(cmd, draw_cmd);
        }
      },
      cmd_variant);
  }
}

void MaterialRenderer::RecordSingleLegacy(RenderCommandList& cmd, const SingleDrawCommand& draw_cmd, const XMMATRIX& view_proj) {
  // Set material instance data (texture indices, etc)
  cmd.SetMaterialData(draw_cmd.material_instance);

  // Set per-object constants for single draw rendering
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

  // Draw mesh (single instance)
  cmd.DrawMesh(draw_cmd.mesh);
}

void MaterialRenderer::RecordInstanceLegacy(RenderCommandList& cmd, const InstanceDrawCommand& draw_cmd) {
  // Set material instance data (texture indices, etc)
  cmd.SetMaterialData(draw_cmd.material_instance);

  // Set minimal ObjectCB for sampler index (instanced rendering gets per-instance data from vertex buffer)
  ObjectCB obj_data = {};
  obj_data.samplerIndex = draw_cmd.material_instance.sampler_index;
  cmd.SetObjectConstants(obj_data);

  // Draw all instances in a single draw call
  cmd.DrawMeshInstanced(draw_cmd.mesh, draw_cmd.instances);
}
