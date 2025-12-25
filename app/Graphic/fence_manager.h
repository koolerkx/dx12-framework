#pragma once

#include <basetsd.h>

#include <mutex>

#include "d3d12.h"
#include "Core/types.h"


class FenceManager {
 public:
  FenceManager() = default;
  ~FenceManager() {
    ShutDown();
  }

  FenceManager(const FenceManager&) = delete;
  FenceManager& operator=(const FenceManager&) = delete;

  bool Initialize(ID3D12Device* device);
  void WaitForGpu(ID3D12CommandQueue* command_queue);

  /// @return value to signal
  UINT64 SignalFence(ID3D12CommandQueue* command_queue);
  void WaitForFenceValue(UINT64 fence_value);

  UINT64 GetCurrentFenceValue() const {
    return fence_value_;
  }

  UINT64 GetCompletedFenceValue() const {
    return fence_ ? fence_->GetCompletedValue() : 0;
  }

  bool IsValid() const {
    return fence_ != nullptr && fence_event_ != nullptr;
  }

 private:
  std::mutex fence_mutex_;
  ComPtr<ID3D12Fence> fence_ = nullptr;
  HANDLE fence_event_ = nullptr;
  UINT64 fence_value_ = 0;

  void ShutDown();
};
