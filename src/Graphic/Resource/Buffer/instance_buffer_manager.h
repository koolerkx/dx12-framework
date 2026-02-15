#pragma once

#include <d3d12.h>

#include <cstdint>
#include <optional>
#include <vector>

#include "Resource/Buffer/gpu_instance_data.h"
#include "Resource/Buffer/instance_buffer_handle.h"
#include "Resource/Buffer/structured_buffer.h"

class InstanceBufferManager {
 public:
  bool Initialize(ID3D12Device* device);

  InstanceBufferHandle Create(uint32_t capacity);
  void Update(InstanceBufferHandle handle, const GPUInstanceData* data, uint32_t count);
  D3D12_GPU_VIRTUAL_ADDRESS GetAddress(InstanceBufferHandle handle) const;
  uint32_t GetCount(InstanceBufferHandle handle) const;

  void Destroy(InstanceBufferHandle handle, uint64_t fence_value);
  void ProcessDeferredFrees(uint64_t completed_fence_value);
  void FlushAllPending();

  void Shutdown();

 private:
  static constexpr uint32_t SLOT_BITS = 16;
  static constexpr uint32_t SLOT_MASK = (1u << SLOT_BITS) - 1;

  struct Entry {
    Graphics::StructuredBuffer<GPUInstanceData> buffer;
    uint32_t count = 0;
  };

  struct Slot {
    std::optional<Entry> entry;
    uint16_t generation = 0;
  };

  struct PendingFree {
    uint32_t slot;
    uint16_t generation;
    uint64_t fence_value;
  };

  static InstanceBufferHandle EncodeHandle(uint32_t slot, uint16_t generation);
  bool DecodeAndValidate(InstanceBufferHandle handle, uint32_t& out_slot) const;

  uint32_t AllocateSlot();
  void ReleaseSlot(uint32_t slot);

  ID3D12Device* device_ = nullptr;
  std::vector<Slot> slots_;
  std::vector<uint32_t> free_list_;
  std::vector<PendingFree> pending_frees_;
};
