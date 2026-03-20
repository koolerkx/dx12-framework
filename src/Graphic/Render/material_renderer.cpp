#include "material_renderer.h"

#include <bit>

#include "Command/render_command_list.h"
#include "Frame/constant_buffers.h"
#include "Framework/Logging/logger.h"
#include "Pipeline/material.h"
#include "Render/shadow_config.h"
#include "Resource/Material/material_descriptor_pool.h"

namespace SortKey {

template <typename T>
uint64_t MaterialFirst(const T& cmd, bool front_to_back) {
  if (!cmd.material) return UINT64_MAX;

  uint32_t rs_key = cmd.material->GetRootSignatureKey() & 0xFFFF;
  uint32_t pso_key = cmd.material->GetPSOKey() & 0xFFFF;
  uint64_t compressed_material = (static_cast<uint64_t>(rs_key) << 48) | (static_cast<uint64_t>(pso_key) << 32);

  uint32_t depth_key = std::bit_cast<uint32_t>(cmd.depth);
  if (!front_to_back) depth_key = ~depth_key;

  return compressed_material | depth_key;
}

template <typename T>
uint64_t DepthFirst(const T& cmd, bool front_to_back) {
  if (!cmd.material) return UINT64_MAX;

  uint32_t depth_key = std::bit_cast<uint32_t>(cmd.depth);
  if (!front_to_back) depth_key = ~depth_key;

  uint32_t rs_key = cmd.material->GetRootSignatureKey() & 0xFFFF;
  uint32_t pso_key = cmd.material->GetPSOKey() & 0xFFFF;
  uint64_t material_key = (static_cast<uint64_t>(rs_key) << 16) | static_cast<uint64_t>(pso_key);

  return (static_cast<uint64_t>(depth_key) << 32) | material_key;
}

template uint64_t MaterialFirst(const ResolvedDrawCommand&, bool);
template uint64_t DepthFirst(const ResolvedDrawCommand&, bool);

}  // namespace SortKey

void MaterialRenderer::BuildResolved(const FramePacket& packet,
  RenderLayer target_layer,
  const DrawCommandResolver::ResolveContext& ctx,
  std::vector<ResolvedDrawCommand>& out_commands) {
  out_commands.clear();

  std::vector<RenderRequest> filtered;
  for (const auto& req : packet.single_requests) {
    if (req.layer == target_layer) filtered.push_back(req);
  }
  DrawCommandResolver::ResolveSingleRequests(ctx, filtered, out_commands);

  std::vector<InternalInstancedRequest> filtered_instanced;
  for (const auto& req : packet.instanced_requests) {
    if (req.request.layer == target_layer) filtered_instanced.push_back(req);
  }
  DrawCommandResolver::ResolveInstancedRequests(ctx, filtered_instanced, packet.instance_data_pool, out_commands);
}

MaterialRenderer::FrameSetup MaterialRenderer::SetupFrameState(const RenderFrameContext& frame,
  const CameraData& camera,
  const LightingConfig& lighting,
  const ShadowConfig& shadow,
  uint32_t screen_width,
  uint32_t screen_height,
  float time,
  const Material* first_material) {
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

  return {std::move(cmd), camera.view_proj, first_material};
}

void MaterialRenderer::RecordResolvedCommands(const RenderFrameContext& frame,
  const std::vector<ResolvedDrawCommand>& commands,
  const CameraData& camera,
  const LightingConfig& lighting,
  const ShadowConfig& shadow,
  uint32_t screen_width,
  uint32_t screen_height,
  float time) {
  if (commands.empty()) return;

  const Material* first_material = nullptr;
  for (const auto& c : commands) {
    if (c.material && c.material->IsValid()) {
      first_material = c.material;
      break;
    }
  }
  if (!first_material) return;

  auto [cmd, view_proj, current_material] =
    SetupFrameState(frame, camera, lighting, shadow, screen_width, screen_height, time, first_material);

  for (const auto& draw_cmd : commands) {
    if (!draw_cmd.material || !draw_cmd.material->IsValid()) continue;

    if (current_material != draw_cmd.material) {
      cmd.SetMaterial(draw_cmd.material);
      current_material = draw_cmd.material;
    }

    RecordResolved(cmd, draw_cmd, view_proj);
  }
}

void MaterialRenderer::RecordResolved(RenderCommandList& cmd, const ResolvedDrawCommand& draw_cmd, const Matrix4& view_proj) {
  ObjectCB obj_data = {};
  obj_data.world = draw_cmd.world_matrix;
  obj_data.worldViewProj = draw_cmd.world_matrix * view_proj;
  if (draw_cmd.instance_count <= 1) {
    obj_data.normalMatrix = draw_cmd.world_matrix.Inverted().Transposed();
  }
  obj_data.color = draw_cmd.color;
  obj_data.uvOffset = draw_cmd.uv_offset;
  obj_data.uvScale = draw_cmd.uv_scale;
  obj_data.flags = draw_cmd.object_flags;
  obj_data.materialDescriptorIndex = draw_cmd.material_handle.IsValid() ? draw_cmd.material_handle.index : 0;

  if (draw_cmd.instance_count > 1) {
    obj_data.flags |= static_cast<uint32_t>(ObjectFlags::Instanced);
    cmd.SetObjectConstants(obj_data);
    cmd.SetInstanceBufferSRV(draw_cmd.instance_buffer_address);
  } else {
    cmd.SetObjectConstants(obj_data);
  }

  if (draw_cmd.custom_data.active) {
    CustomCB custom_cb = {};
    memcpy(custom_cb.data, draw_cmd.custom_data.data.data(), sizeof(float) * 20);
    cmd.SetCustomConstants(custom_cb);
  }

  auto vbv = draw_cmd.geometry.vbv;
  auto ibv = draw_cmd.geometry.ibv;
  cmd.GetNative()->IASetVertexBuffers(0, 1, &vbv);
  cmd.GetNative()->IASetIndexBuffer(&ibv);
  cmd.GetNative()->DrawIndexedInstanced(
    draw_cmd.geometry.index_count, draw_cmd.instance_count, draw_cmd.geometry.index_offset, draw_cmd.geometry.vertex_offset, 0);
}
