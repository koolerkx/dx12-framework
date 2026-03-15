#include "material_renderer.h"

#include <bit>

#include "Command/render_command_list.h"
#include "Frame/constant_buffers.h"
#include "Framework/Logging/logger.h"
#include "Pipeline/material.h"
#include "Render/shadow_config.h"
#include "Resource/Material/material_descriptor_pool.h"

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
  const LightingConfig& lighting,
  const ShadowConfig& shadow,
  uint32_t screen_width,
  uint32_t screen_height,
  float time) {
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

  FrameCB frame_cb_data = {};
  frame_cb_data.view = camera.view;
  frame_cb_data.proj = camera.proj;
  frame_cb_data.viewProj = camera.view_proj;
  frame_cb_data.invView = camera.inv_view;
  frame_cb_data.invProj = camera.inv_proj;
  frame_cb_data.cameraPos = camera.position;
  frame_cb_data.screenSize = Vector2(static_cast<float>(screen_width), static_cast<float>(screen_height));
  frame_cb_data.time = time;
  cmd.SetFrameConstants(frame_cb_data);

  LightingCB lighting_cb = {};
  lighting_cb.lightDirection = lighting.direction;
  lighting_cb.lightIntensity = lighting.intensity;
  lighting_cb.directionalColor = lighting.directional_color;
  lighting_cb.ambientIntensity = lighting.ambient_intensity;
  lighting_cb.ambientColor = lighting.ambient_color;
  lighting_cb.pointLightCount = frame.point_light_count;
  lighting_cb.ssaoSrvIndex = frame.ssao_srv_index;
  cmd.SetLightingConstants(lighting_cb);
  cmd.SetPointLightSRV(frame.point_light_srv);

  ShadowCB shadow_cb = {};
  if (shadow.enabled && frame.shadow_data) {
    auto* sd = frame.shadow_data;
    for (uint32_t i = 0; i < ShadowCascadeConfig::MAX_CASCADES; ++i) {
      shadow_cb.lightViewProj[i] = sd->light_view_proj[i];
      shadow_cb.shadowMapIndex[i] = sd->shadow_map_srv_index[i];
      shadow_cb.cascadeDepthBias[i] = shadow.cascade_depth_bias[i];
      shadow_cb.cascadeNormalBias[i] = shadow.cascade_normal_bias[i];
      shadow_cb.cascadeSplitDistances[i] = sd->cascade_split_distances[i];
    }
    shadow_cb.shadowAlgorithm = static_cast<uint32_t>(shadow.algorithm);
    shadow_cb.shadowMapResolution = sd->shadow_map_resolution;
    shadow_cb.cascadeCount = sd->cascade_count;
    shadow_cb.cascadeBlendRange = shadow.cascade_blend_range;
    shadow_cb.shadowColor = shadow.shadow_color;
    shadow_cb.lightSize = shadow.light_size;
  }
  cmd.SetShadowConstants(shadow_cb);

  if (frame.material_descriptor_pool) {
    cmd.SetMaterialDescriptorSRV(frame.material_descriptor_pool->GetBufferAddress());
  }

  Matrix4 view_proj = camera.view_proj;
  const Material* current_material = first_material;

  bool unified_buffers_bound = false;
  MeshBufferPool* mesh_pool = frame.mesh_buffer_pool;

  for (const auto& draw_cmd : commands) {
    if (!draw_cmd.material || !draw_cmd.material->IsValid()) {
      continue;
    }
    if (!draw_cmd.mesh && !draw_cmd.UsesBindlessMesh()) {
      continue;
    }

    if (current_material != draw_cmd.material) {
      cmd.SetMaterial(draw_cmd.material);
      current_material = draw_cmd.material;
    }

    if (draw_cmd.UsesBindlessMesh()) {
      if (!unified_buffers_bound) {
        cmd.BindUnifiedMeshBuffers(mesh_pool);
        cmd.SetMeshDescriptorSRV(mesh_pool->GetDescriptorBufferAddress());
        unified_buffers_bound = true;
      }
      if (draw_cmd.IsStructuredInstanced()) {
        RecordBindlessStructuredInstanced(cmd, draw_cmd, view_proj, shadow.enabled, mesh_pool);
      } else {
        RecordBindlessSingle(cmd, draw_cmd, view_proj, shadow.enabled, mesh_pool);
      }
    } else if (draw_cmd.IsStructuredInstanced()) {
      unified_buffers_bound = false;
      if (!draw_cmd.material->SupportsStructuredInstancing()) {
        Logger::LogFormat(LogLevel::Error,
          LogCategory::Graphic,
          Logger::Here(),
          "DrawCommand has structured instance data but material '{}' does not support structured instancing",
          draw_cmd.material->GetName());
        continue;
      }
      RecordStructuredInstanced(cmd, draw_cmd, view_proj, shadow.enabled);
    } else if (draw_cmd.IsInstanced()) {
      unified_buffers_bound = false;
      RecordInstanced(cmd, draw_cmd);
    } else {
      unified_buffers_bound = false;
      RecordSingle(cmd, draw_cmd, view_proj, shadow.enabled);
    }
  }
}

void MaterialRenderer::RecordSingle(RenderCommandList& cmd, const DrawCommand& draw_cmd, const Matrix4& view_proj, bool shadow_enabled) {
  ObjectCB obj_data = {};
  Matrix4 world = draw_cmd.world_matrix;
  Matrix4 wvp = world * view_proj;
  obj_data.world = world;
  obj_data.worldViewProj = wvp;
  obj_data.normalMatrix = world.Inverted().Transposed();
  obj_data.color = draw_cmd.color;
  obj_data.uvOffset = draw_cmd.uv_offset;
  obj_data.uvScale = draw_cmd.uv_scale;
  obj_data.flags = flags::If(HasTag(draw_cmd.tags, RenderTag::Lit), ObjectFlags::Lit) |
                   flags::If(draw_cmd.layer == RenderLayer::Opaque, ObjectFlags::Opaque) |
                   flags::If(shadow_enabled && HasTag(draw_cmd.tags, RenderTag::ReceiveShadow), ObjectFlags::ReceiveShadow);
  obj_data.materialDescriptorIndex = draw_cmd.material_handle.index;
  cmd.SetObjectConstants(obj_data);

  if (draw_cmd.has_custom_data) {
    CustomCB custom_cb = {};
    memcpy(custom_cb.data, draw_cmd.custom_data.data(), sizeof(float) * 20);
    cmd.SetCustomConstants(custom_cb);
  }

  cmd.DrawMesh(draw_cmd.mesh);
}

void MaterialRenderer::RecordStructuredInstanced(
  RenderCommandList& cmd, const DrawCommand& draw_cmd, const Matrix4& view_proj, bool shadow_enabled) {
  ObjectCB obj_data = {};
  obj_data.color = draw_cmd.color;
  obj_data.uvScale = {1.0f, 1.0f};
  uint32_t obj_flags = static_cast<uint32_t>(ObjectFlags::Instanced);
  if (HasTag(draw_cmd.tags, RenderTag::Lit)) obj_flags |= static_cast<uint32_t>(ObjectFlags::Lit);
  if (draw_cmd.layer == RenderLayer::Opaque) obj_flags |= static_cast<uint32_t>(ObjectFlags::Opaque);
  if (shadow_enabled && HasTag(draw_cmd.tags, RenderTag::ReceiveShadow)) obj_flags |= static_cast<uint32_t>(ObjectFlags::ReceiveShadow);
  obj_data.flags = obj_flags;
  obj_data.materialDescriptorIndex = draw_cmd.material_handle.index;
  cmd.SetObjectConstants(obj_data);

  cmd.SetInstanceBufferSRV(draw_cmd.instance_buffer_address);

  const Mesh* mesh = draw_cmd.mesh;
  auto vbv = mesh->GetVertexBuffer().GetView();
  auto ibv = mesh->GetIndexBuffer().GetView();
  cmd.GetNative()->IASetVertexBuffers(0, 1, &vbv);
  cmd.GetNative()->IASetIndexBuffer(&ibv);

  for (size_t i = 0; i < mesh->GetSubMeshCount(); ++i) {
    const auto& sub = mesh->GetSubMesh(i);
    cmd.GetNative()->DrawIndexedInstanced(sub.indexCount, draw_cmd.instance_count, sub.startIndexLocation, sub.baseVertexLocation, 0);
  }
}

void MaterialRenderer::RecordInstanced(RenderCommandList& cmd, const DrawCommand& draw_cmd) {
  ObjectCB obj_data = {};
  obj_data.materialDescriptorIndex = draw_cmd.material_handle.index;
  cmd.SetObjectConstants(obj_data);

  if (draw_cmd.has_custom_data) {
    CustomCB custom_cb = {};
    memcpy(custom_cb.data, draw_cmd.custom_data.data(), sizeof(float) * 20);
    cmd.SetCustomConstants(custom_cb);
  }

  cmd.DrawMeshInstanced(draw_cmd.mesh, draw_cmd.instances);
}

void MaterialRenderer::RecordBindlessSingle(
  RenderCommandList& cmd, const DrawCommand& draw_cmd, const Matrix4& view_proj, bool shadow_enabled, MeshBufferPool* pool) {
  ObjectCB obj_data = {};
  Matrix4 world = draw_cmd.world_matrix;
  Matrix4 wvp = world * view_proj;
  obj_data.world = world;
  obj_data.worldViewProj = wvp;
  obj_data.normalMatrix = world.Inverted().Transposed();
  obj_data.color = draw_cmd.color;
  obj_data.uvOffset = draw_cmd.uv_offset;
  obj_data.uvScale = draw_cmd.uv_scale;
  obj_data.flags = flags::If(HasTag(draw_cmd.tags, RenderTag::Lit), ObjectFlags::Lit) |
                   flags::If(draw_cmd.layer == RenderLayer::Opaque, ObjectFlags::Opaque) |
                   flags::If(shadow_enabled && HasTag(draw_cmd.tags, RenderTag::ReceiveShadow), ObjectFlags::ReceiveShadow);
  obj_data.materialDescriptorIndex = draw_cmd.material_handle.index;
  cmd.SetObjectConstants(obj_data);

  if (draw_cmd.has_custom_data) {
    CustomCB custom_cb = {};
    memcpy(custom_cb.data, draw_cmd.custom_data.data(), sizeof(float) * 20);
    cmd.SetCustomConstants(custom_cb);
  }

  const MeshDescriptor* desc = pool->GetDescriptor(draw_cmd.mesh_handle);
  if (desc) {
    cmd.DrawBindlessMesh(*desc);
  }
}

void MaterialRenderer::RecordBindlessStructuredInstanced(
  RenderCommandList& cmd, const DrawCommand& draw_cmd, const Matrix4& view_proj, bool shadow_enabled, MeshBufferPool* pool) {
  ObjectCB obj_data = {};
  obj_data.color = draw_cmd.color;
  obj_data.uvScale = {1.0f, 1.0f};
  uint32_t obj_flags = static_cast<uint32_t>(ObjectFlags::Instanced);
  if (HasTag(draw_cmd.tags, RenderTag::Lit)) obj_flags |= static_cast<uint32_t>(ObjectFlags::Lit);
  if (draw_cmd.layer == RenderLayer::Opaque) obj_flags |= static_cast<uint32_t>(ObjectFlags::Opaque);
  if (shadow_enabled && HasTag(draw_cmd.tags, RenderTag::ReceiveShadow)) obj_flags |= static_cast<uint32_t>(ObjectFlags::ReceiveShadow);
  obj_data.flags = obj_flags;
  obj_data.materialDescriptorIndex = draw_cmd.material_handle.index;
  cmd.SetObjectConstants(obj_data);

  cmd.SetInstanceBufferSRV(draw_cmd.instance_buffer_address);

  const MeshDescriptor* desc = pool->GetDescriptor(draw_cmd.mesh_handle);
  if (desc) {
    cmd.DrawBindlessMesh(*desc, draw_cmd.instance_count);
  }
}
