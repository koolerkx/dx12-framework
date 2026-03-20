#pragma once

#include <span>
#include <vector>

#include "Frame/frame_packet.h"
#include "Frame/resolved_draw_command.h"

class DynamicUploadBuffer;
class MaterialManager;
class MeshBufferPool;

class DrawCommandResolver {
 public:
  struct ResolveContext {
    MaterialManager* material_manager;
    MeshBufferPool* mesh_buffer_pool;
    DynamicUploadBuffer* instance_allocator;
    bool shadow_enabled;
  };

  static void ResolveSingleRequests(
    const ResolveContext& ctx,
    std::span<const RenderRequest> requests,
    std::vector<ResolvedDrawCommand>& out);

  static void ResolveInstancedRequests(
    const ResolveContext& ctx,
    std::span<const InternalInstancedRequest> requests,
    const std::vector<std::byte>& instance_data_pool,
    std::vector<ResolvedDrawCommand>& out);

 private:
  static MeshGeometry ResolveMeshGeometry(MeshBufferPool* pool, MeshHandle handle);
  static uint32_t BuildObjectFlags(RenderTagMask tags, RenderLayer layer, bool shadow_enabled);
};
