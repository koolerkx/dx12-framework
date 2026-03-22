/**
 * @file mesh_service.h
 * @brief Interface for GPU mesh allocation, decoupled from GPU implementation.
 */
#pragma once

#include "Framework/Render/mesh_data.h"
#include "Framework/Render/render_handles.h"

class IMeshService {
 public:
  virtual ~IMeshService() = default;
  virtual MeshAllocation Allocate(const MeshData& data) = 0;
  virtual void Free(MeshHandle handle) = 0;
};
