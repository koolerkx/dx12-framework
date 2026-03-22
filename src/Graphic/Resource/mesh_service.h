/**
 * @file mesh_service.h
 * @brief MeshService implementation delegating to MeshBufferPool.
 */
#pragma once

#include "Framework/Asset/mesh_service.h"

class MeshBufferPool;

class MeshService : public IMeshService {
 public:
  explicit MeshService(MeshBufferPool& mesh_buffer_pool);

  MeshAllocation Allocate(const MeshData& data) override;
  void Free(MeshHandle handle) override;

 private:
  MeshBufferPool& mesh_buffer_pool_;
};
