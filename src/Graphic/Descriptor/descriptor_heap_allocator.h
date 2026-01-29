#pragma once
#include <d3d12.h>

#include <cstdint>
#include <deque>
#include <mutex>
#include <vector>

#include "Core/types.h"

class DescriptorHeapAllocator {
 public:
  struct Allocation {
    D3D12_CPU_DESCRIPTOR_HANDLE cpu;
    D3D12_GPU_DESCRIPTOR_HANDLE gpu;
    uint32_t index;  // Index relative to the global heap start

    bool IsValid() const {
      return cpu.ptr != 0;
    }
  };

  struct FreeBlock {
    uint32_t offset;
    uint32_t size;
    bool operator<(const FreeBlock& other) const {
      return offset < other.offset;
    }
  };

  struct DeferredFree {
    uint32_t offset;
    uint32_t count;
    uint64_t fence_value;
  };

  DescriptorHeapAllocator() = default;

  // Mode 1: Create its own heap (Used for RTV/DSV)
  bool Initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t capacity, bool shader_visible);

  // Mode 2: Manage a range of an existing heap (Used for SRV Global Heap)
  void InitializeSubAllocation(ID3D12DescriptorHeap* heap, uint32_t offset, uint32_t capacity, uint32_t descriptor_size);

  Allocation Allocate(uint32_t count = 1);

  // Immediate free - caller guarantees GPU is done with these descriptors
  void FreeImmediate(uint32_t offset, uint32_t count);

  // Periodic cleanup to reduce fragmentation
  void CoalesceFreeBlocks();

  void Reset();  // Only resets the allocation pointer, does not free memory

  ID3D12DescriptorHeap* GetHeap() const {
    return heap_.Get();
  }

 private:
  std::mutex alloc_mutex_;
  ComPtr<ID3D12DescriptorHeap> heap_ = nullptr;    // Only set if we own the heap
  ID3D12DescriptorHeap* external_heap_ = nullptr;  // Set if sub-allocating

  D3D12_CPU_DESCRIPTOR_HANDLE start_cpu_ = {};
  D3D12_GPU_DESCRIPTOR_HANDLE start_gpu_ = {};
  uint32_t descriptor_size_ = 0;

  uint32_t capacity_ = 0;
  uint32_t allocated_ = 0;

  // For calculating absolute handles
  D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(uint32_t offset_index) const;
  D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(uint32_t offset_index) const;

  std::vector<FreeBlock> free_list_;
  void AddToFreeList(uint32_t offset, uint32_t count);
};
