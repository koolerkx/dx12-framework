#pragma once

#include <cassert>
#include <vector>

#include "buffer.h"
#include "fence_manager.h"

template <typename T>
class PerFrameConstantBuffer {
 public:
  PerFrameConstantBuffer() = default;
  ~PerFrameConstantBuffer() = default;

  // Initialize N buffers (N = Frame Count)
  bool Initialize(ID3D12Device* device, uint32_t frame_count) {
    assert(device != nullptr);
    assert(frame_count > 0);

    frame_count_ = frame_count;
    buffers_.resize(frame_count);
    frame_fence_values_.resize(frame_count, 0);

    for (uint32_t i = 0; i < frame_count; ++i) {
      if (!buffers_[i].Create(device)) {
        return false;
      }
    }
    return true;
  }

  // Get the buffer for the specific frame index
  ConstantBuffer<T>& GetBuffer(uint32_t frame_index) {
    assert(frame_index < frame_count_);
    return buffers_[frame_index];
  }

  // Ensure the GPU has finished reading this frame's buffer before we write to it again
  void WaitForFrame(uint32_t frame_index, FenceManager* fence_manager) {
    assert(frame_index < frame_count_);

    uint64_t fence_value = frame_fence_values_[frame_index];
    if (fence_value != 0) {
      // Assume FenceManager has a method like WaitForFenceValue
      fence_manager->WaitForFenceValue(fence_value);
    }
  }

  // Call this after ExecuteCommandLists to record when this buffer will be free
  void MarkFrameSubmitted(uint32_t frame_index, uint64_t fence_value) {
    assert(frame_index < frame_count_);
    frame_fence_values_[frame_index] = fence_value;
  }

 private:
  std::vector<ConstantBuffer<T>> buffers_;
  std::vector<uint64_t> frame_fence_values_;
  uint32_t frame_count_ = 0;
};
