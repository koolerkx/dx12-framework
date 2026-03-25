/**
 * @file object_data_buffer.h
 * @brief Per-frame CPU staging + GPU upload for unified ObjectData StructuredBuffer.
 * @note expected the data will be clear and updated every frame
 */
#pragma once
#include <d3d12.h>

#include <cstdint>
#include <vector>

#include "Frame/object_data.h"

class DynamicUploadBuffer;

class ObjectDataBuffer {
 public:
  uint32_t Append(const ObjectData& data);
  void Upload(DynamicUploadBuffer& allocator);
  void Reset();

  D3D12_GPU_VIRTUAL_ADDRESS GetBufferAddress() const {
    return gpu_address_;
  }
  uint32_t GetObjectCount() const {
    return static_cast<uint32_t>(staging_.size());
  }

 private:
  std::vector<ObjectData> staging_;
  D3D12_GPU_VIRTUAL_ADDRESS gpu_address_ = 0;
};
