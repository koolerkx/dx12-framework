#pragma once

#include <d3d12.h>

#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <vector>

#include "Core/types.h"

class DescriptorHeapAllocator {
 public:
  struct Allocation {
    D3D12_CPU_DESCRIPTOR_HANDLE cpu;
    D3D12_GPU_DESCRIPTOR_HANDLE gpu;
    uint32_t index;
    uint32_t count;

    bool IsValid() const {
      return cpu.ptr != 0;
    }
    bool IsShaderVisible() const {
      return gpu.ptr != 0;
    }
  };

  DescriptorHeapAllocator() = default;
  ~DescriptorHeapAllocator() = default;

  DescriptorHeapAllocator(const DescriptorHeapAllocator&) = delete;
  DescriptorHeapAllocator& operator=(const DescriptorHeapAllocator&) = delete;

  bool Initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t capacity, bool shader_visible = false);
  Allocation Allocate(uint32_t count = 1);
  void Free(const Allocation& allocation);
  void Reset();

  ID3D12DescriptorHeap* GetHeap() const {
    return heap_.Get();
  }
  D3D12_DESCRIPTOR_HEAP_TYPE GetType() const {
    return type_;
  }

  // Statistic
  uint32_t GetCapacity() const {
    return capacity_;
  }
  uint32_t GetAllocated() const {
    return allocated_;
  }
  uint32_t GetAvailable() const {
    return capacity_ - allocated_;
  }

 private:
  std::mutex alloc_mutex_;

  ComPtr<ID3D12DescriptorHeap> heap_ = nullptr;
  D3D12_DESCRIPTOR_HEAP_TYPE type_ = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  uint32_t descriptor_size_ = 0;
  uint32_t capacity_ = 0;
  uint32_t allocated_ = 0;
  bool shader_visible_ = false;

  D3D12_CPU_DESCRIPTOR_HANDLE heap_start_cpu_ = D3D12_CPU_DESCRIPTOR_HANDLE{0};
  D3D12_GPU_DESCRIPTOR_HANDLE heap_start_gpu_ = D3D12_GPU_DESCRIPTOR_HANDLE{0};

  std::priority_queue<uint32_t, std::vector<uint32_t>, std::greater<uint32_t>> free_list_ = {};

  D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(uint32_t index) const;
  D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(uint32_t index) const;
};
