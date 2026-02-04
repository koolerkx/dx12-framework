#include "frame_synchronizer.h"

#include "Framework/Logging/logger.h"

namespace gfx {

std::unique_ptr<FrameSynchronizer> FrameSynchronizer::Create(ID3D12Device* device) {
  auto synchronizer = std::unique_ptr<FrameSynchronizer>(new FrameSynchronizer());
  if (!synchronizer->Initialize(device)) {
    return nullptr;
  }
  return synchronizer;
}

bool FrameSynchronizer::Initialize(ID3D12Device* device) {
  if (!fence_manager_.Initialize(device)) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to initialize FenceManager");
    return false;
  }

  frame_fence_values_.fill(0);
  Logger::LogFormat(LogLevel::Info, LogCategory::Graphic, Logger::Here(), "FrameSynchronizer initialized");
  return true;
}

void FrameSynchronizer::WaitForFrame(uint32_t frame_index) {
  if (frame_index >= kMaxFramesInFlight) {
    Logger::LogFormat(
      LogLevel::Error, LogCategory::Graphic, Logger::Here(), "Invalid frame index: {} (max: {})", frame_index, kMaxFramesInFlight - 1);
    return;
  }

  uint64_t fence_value = frame_fence_values_[frame_index];
  if (fence_value != 0) {
    fence_manager_.WaitForFenceValue(fence_value);
  }
}

void FrameSynchronizer::WaitForGpuIdle(ID3D12CommandQueue* queue) {
  fence_manager_.WaitForGpu(queue);
}

uint64_t FrameSynchronizer::SignalFrame(ID3D12CommandQueue* queue, uint32_t frame_index) {
  if (frame_index >= kMaxFramesInFlight) {
    Logger::LogFormat(
      LogLevel::Error, LogCategory::Graphic, Logger::Here(), "Invalid frame index: {} (max: {})", frame_index, kMaxFramesInFlight - 1);
    return 0;
  }

  uint64_t fence_value = fence_manager_.SignalFence(queue);
  frame_fence_values_[frame_index] = fence_value;
  return fence_value;
}

uint64_t FrameSynchronizer::GetCompletedValue() const {
  return fence_manager_.GetCompletedFenceValue();
}

uint64_t FrameSynchronizer::GetPendingValue(uint32_t frame_index) const {
  if (frame_index >= kMaxFramesInFlight) {
    return 0;
  }
  return frame_fence_values_[frame_index];
}

uint64_t FrameSynchronizer::GetCurrentFenceValue() const {
  return fence_manager_.GetCurrentFenceValue();
}

ID3D12Fence* FrameSynchronizer::GetFence() const {
  return fence_manager_.GetFence();
}

bool FrameSynchronizer::IsValid() const {
  return fence_manager_.IsValid();
}

}  // namespace gfx
