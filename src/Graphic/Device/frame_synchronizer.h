#pragma once

#include <d3d12.h>

#include <array>
#include <cstdint>
#include <memory>

#include "fence_manager.h"

namespace gfx {

class FrameSynchronizer {
 public:
  static constexpr uint32_t kMaxFramesInFlight = 2;

  [[nodiscard]] static std::unique_ptr<FrameSynchronizer> Create(ID3D12Device* device);

  FrameSynchronizer(const FrameSynchronizer&) = delete;
  FrameSynchronizer& operator=(const FrameSynchronizer&) = delete;
  ~FrameSynchronizer() = default;

  void WaitForFrame(uint32_t frame_index);
  void WaitForGpuIdle(ID3D12CommandQueue* queue);

  uint64_t SignalFrame(ID3D12CommandQueue* queue, uint32_t frame_index);
  uint64_t GetCompletedValue() const;
  uint64_t GetPendingValue(uint32_t frame_index) const;
  uint64_t GetCurrentFenceValue() const;

  ID3D12Fence* GetFence() const;
  bool IsValid() const;

  FenceManager& GetFenceManager() {
    return fence_manager_;
  }

 private:
  FrameSynchronizer() = default;
  bool Initialize(ID3D12Device* device);

  FenceManager fence_manager_;
  std::array<uint64_t, kMaxFramesInFlight> frame_fence_values_{};
};

}  // namespace gfx
