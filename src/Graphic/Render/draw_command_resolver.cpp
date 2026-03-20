#include "draw_command_resolver.h"

#include <cassert>

#include "Frame/constant_buffers.h"
#include "Frame/dynamic_upload_buffer.h"
#include "Pipeline/material_manager.h"
#include "Pipeline/shader_types.h"
#include "Resource/Mesh/mesh_buffer_pool.h"
#include "Resource/Mesh/mesh_descriptor.h"

void DrawCommandResolver::ResolveSingleRequests(
  const ResolveContext& ctx, std::span<const RenderRequest> requests, std::vector<ResolvedDrawCommand>& out) {
  for (const auto& req : requests) {
    const Material* material =
      ctx.material_manager->GetOrCreateMaterial(static_cast<Graphics::ShaderId>(req.shader_id), req.render_settings);
    if (!material) continue;

    MeshGeometry geo = ResolveMeshGeometry(ctx.mesh_buffer_pool, req.mesh);
    if (geo.index_count == 0) continue;

    ResolvedDrawCommand cmd;
    cmd.material = material;
    cmd.geometry = geo;
    cmd.world_matrix = req.world_matrix;
    cmd.color = req.color;
    cmd.uv_offset = req.uv_offset;
    cmd.uv_scale = req.uv_scale;
    cmd.object_flags = BuildObjectFlags(req.tags, req.layer, ctx.shadow_enabled);
    cmd.instance_count = 1;
    cmd.material_handle = req.material;
    cmd.layer = req.layer;
    cmd.tags = req.tags;
    cmd.depth = req.depth;
    cmd.depth_write = req.render_settings.depth_write;
    cmd.custom_data = req.custom_data;
    out.push_back(cmd);
  }
}

void DrawCommandResolver::ResolveInstancedRequests(const ResolveContext& ctx,
  std::span<const InternalInstancedRequest> requests,
  const std::vector<std::byte>& instance_data_pool,
  std::vector<ResolvedDrawCommand>& out) {
  for (const auto& internal : requests) {
    const auto& req = internal.request;
    const auto& data_ref = internal.instance_data;
    if (!data_ref.IsValid()) continue;
    assert(data_ref.offset + data_ref.size <= instance_data_pool.size());

    const Material* material =
      ctx.material_manager->GetOrCreateMaterial(static_cast<Graphics::ShaderId>(req.shader_id), req.render_settings);
    if (!material) continue;

    MeshGeometry geo = ResolveMeshGeometry(ctx.mesh_buffer_pool, req.mesh);
    if (geo.index_count == 0) continue;

    ResolvedDrawCommand cmd;
    cmd.material = material;
    cmd.geometry = geo;
    cmd.material_handle = req.material;
    cmd.layer = req.layer;
    cmd.tags = req.tags;
    cmd.depth = req.depth;
    cmd.depth_write = req.render_settings.depth_write;
    cmd.custom_data = req.custom_data;

    if (data_ref.count == 1) {
      const auto* instance = reinterpret_cast<const InstanceData*>(instance_data_pool.data() + data_ref.offset);
      cmd.world_matrix = instance->world;
      cmd.color = {
        instance->color.x * req.color.x, instance->color.y * req.color.y, instance->color.z * req.color.z, instance->color.w * req.color.w};
      cmd.uv_offset = instance->uv_offset;
      cmd.uv_scale = instance->uv_scale;
      cmd.object_flags = BuildObjectFlags(req.tags, req.layer, ctx.shadow_enabled);
      cmd.instance_count = 1;
    } else {
      auto alloc = ctx.instance_allocator->Allocate(data_ref.size);
      if (!alloc.cpu_ptr) continue;
      memcpy(alloc.cpu_ptr, instance_data_pool.data() + data_ref.offset, data_ref.size);

      // Per-instance fields (world, uv_offset, uv_scale) live in the
      // StructuredBuffer and are read by VS via g_InstanceBuffer[instanceID].
      // color is set from req because it carries per-submesh material tint
      // (e.g. base_color_factor), which is distinct from per-instance color.
      cmd.world_matrix = Math::Matrix4::Identity;
      cmd.color = req.color;
      cmd.object_flags = BuildObjectFlags(req.tags, req.layer, ctx.shadow_enabled) | static_cast<uint32_t>(ObjectFlags::Instanced);
      cmd.instance_count = data_ref.count;
      cmd.instance_buffer_address = alloc.gpu_ptr;
    }

    out.push_back(cmd);
  }
}

MeshGeometry DrawCommandResolver::ResolveMeshGeometry(MeshBufferPool* pool, MeshHandle handle) {
  MeshGeometry geo{};
  if (!pool || !handle.IsValid()) return geo;

  const MeshDescriptor* desc = pool->GetDescriptor(handle);
  if (!desc) return geo;

  VertexLayout layout = pool->GetVertexLayout(handle);
  geo.vbv = (layout == VertexLayout::Sprite) ? pool->GetSpriteVertexBufferView() : pool->GetVertexBufferView();
  geo.ibv = pool->GetIndexBufferView();
  geo.index_count = desc->index_count;
  geo.index_offset = desc->index_offset;
  geo.vertex_offset = desc->vertex_offset;
  return geo;
}

uint32_t DrawCommandResolver::BuildObjectFlags(RenderTagMask tags, RenderLayer layer, bool shadow_enabled) {
  return flags::If(HasTag(tags, RenderTag::Lit), ObjectFlags::Lit) | flags::If(layer == RenderLayer::Opaque, ObjectFlags::Opaque) |
         flags::If(shadow_enabled && HasTag(tags, RenderTag::ReceiveShadow), ObjectFlags::ReceiveShadow);
}
