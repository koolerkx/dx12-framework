#include "fence_manager.h"

#include <cassert>
#include <iostream>

bool FenceManager::Initialize(ID3D12Device* device) {
  assert(device != nullptr);

  HRESULT hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
  if (FAILED(hr)) {
    std::cerr << "[FenceManager] Failed to create fence." << std::endl;
    return false;
  }

  fence_value_ = 1;

  fence_event_ = CreateEvent(nullptr, false, false, nullptr);
  if (!fence_event_) {
    std::cerr << "[FenceManager] Failed to create fence event." << std::endl;
    return false;
  }

  return true;
}

UINT64 FenceManager::SignalFence(ID3D12CommandQueue* command_queue) {
  assert(command_queue != nullptr);
  assert(fence_ != nullptr);

  std::lock_guard<std::mutex> lock(fence_mutex_);
  UINT64 value_to_signal = fence_value_;

  command_queue->Signal(fence_.Get(), value_to_signal);
  fence_value_++;

  return value_to_signal;
}

void FenceManager::WaitForFenceValue(UINT64 fence_value) {
  assert(fence_ != nullptr);
  assert(fence_event_ != nullptr);

  if (fence_->GetCompletedValue() < fence_value) {
    fence_->SetEventOnCompletion(fence_value, fence_event_);
    WaitForSingleObject(fence_event_, INFINITE);
  }
}

void FenceManager::WaitForGpu(ID3D12CommandQueue* command_queue) {
  assert(command_queue != nullptr);
  assert(fence_ != nullptr);
  assert(fence_event_ != nullptr);

  const UINT64 current_fence_value = fence_value_;
  command_queue->Signal(fence_.Get(), current_fence_value);
  fence_value_++;

  if (fence_->GetCompletedValue() < current_fence_value) {
    fence_->SetEventOnCompletion(current_fence_value, fence_event_);
    WaitForSingleObject(fence_event_, INFINITE);
  }
}

void FenceManager::ShutDown() {
  if (fence_event_) {
    CloseHandle(fence_event_);
    fence_event_ = nullptr;
  }
  fence_.Reset();
}
