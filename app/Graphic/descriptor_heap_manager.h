#pragma once

#include <cstdint>

#include "d3d12.h"
#include "descriptor_heap_allocator.h"

class DescriptorHeapManager {
 public:
  DescriptorHeapManager() = default;
  ~DescriptorHeapManager() = default;

  DescriptorHeapManager(const DescriptorHeapManager&) = delete;
  DescriptorHeapManager& operator=(const DescriptorHeapManager&) = delete;

  bool Initalize(ID3D12Device* device);
  void BeginFrame();
  void SetDescriptorHeaps(ID3D12GraphicsCommandList* command_list);

  DescriptorHeapAllocator& GetRtvAllocator() {
    return rtv_heap_;
  }
  DescriptorHeapAllocator& GetDsvAllocator() {
    return dsv_heap_;
  }
  DescriptorHeapAllocator& GetSrvAllocator() {
    return srv_heap_;
  }
  DescriptorHeapAllocator& GetSamplerAllocator() {
    return sampler_heap_;
  }

  void PrintStats() const;

 private:
  std::mutex begin_frame_mutex_;

  DescriptorHeapAllocator rtv_heap_;
  DescriptorHeapAllocator dsv_heap_;

  DescriptorHeapAllocator srv_heap_;
  DescriptorHeapAllocator sampler_heap_;

  struct Config {
    uint32_t rtv_capacity = 256;
    uint32_t dsv_capacity = 64;

    uint32_t srv_capacity = 4096;
    uint32_t sampler_capacity = 256;
  };

  Config config_;
};
