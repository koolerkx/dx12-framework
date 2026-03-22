#include "mesh_service.h"

#include <variant>

#include "Framework/Logging/logger.h"
#include "Framework/Render/vertex_data.h"
#include "Mesh/mesh_buffer_pool.h"
#include "Pipeline/vertex_types.h"

static_assert(sizeof(VertexData::ModelVertex) == sizeof(Graphics::Vertex::ModelVertex));
static_assert(sizeof(VertexData::SpriteVertex) == sizeof(Graphics::Vertex::SpriteVertex));
static_assert(alignof(VertexData::ModelVertex) == alignof(Graphics::Vertex::ModelVertex));
static_assert(alignof(VertexData::SpriteVertex) == alignof(Graphics::Vertex::SpriteVertex));

MeshService::MeshService(MeshBufferPool& mesh_buffer_pool) : mesh_buffer_pool_(mesh_buffer_pool) {
}

MeshAllocation MeshService::Allocate(const MeshData& data) {
  return std::visit(
    [&](const auto& verts) -> MeshAllocation {
      using VertexType = typename std::decay_t<decltype(verts)>::value_type;

      if (verts.empty()) {
        Logger::LogFormat(LogLevel::Error, LogCategory::Core, Logger::Here(), "[MeshService] Allocate() called with empty vertex data");
        return {};
      }

      if constexpr (std::is_same_v<VertexType, VertexData::ModelVertex>) {
        auto gpu_verts = std::span<const Graphics::Vertex::ModelVertex>(
          reinterpret_cast<const Graphics::Vertex::ModelVertex*>(verts.data()), verts.size());
        return mesh_buffer_pool_.Allocate(gpu_verts, std::span<const uint32_t>(data.indices));
      } else {
        auto gpu_verts = std::span<const Graphics::Vertex::SpriteVertex>(
          reinterpret_cast<const Graphics::Vertex::SpriteVertex*>(verts.data()), verts.size());
        return mesh_buffer_pool_.Allocate(gpu_verts, std::span<const uint32_t>(data.indices));
      }
    },
    data.vertices);
}

void MeshService::Free(MeshHandle handle) {
  mesh_buffer_pool_.Free(handle);
}
