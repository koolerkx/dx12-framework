#pragma once
#include <memory>
#include <vector>

#include "Descriptor/descriptor_heap_allocator.h"

struct DescriptorHeapConfig {
  uint32_t rtv_capacity = 512;
  uint32_t dsv_capacity = 512;
  uint32_t srv_static_size = 100000;
  uint32_t srv_frame_dynamic_size = 10000;
  // uint32_t srv_capacity = 4096;      // not in use
  // uint32_t sampler_capacity = 2048;  // not in use
};

class DescriptorHeapManager {
 public:
  bool Initialize(ID3D12Device* device, uint32_t frame_count, const DescriptorHeapConfig& config = {});

  // Call at start of frame to reset that frame's dynamic region
  void BeginFrame(uint32_t frame_index);

  // Global Bind (Always binds the same global heap)
  void SetDescriptorHeaps(ID3D12GraphicsCommandList* cmdList);

  DescriptorHeapAllocator& GetRtvAllocator() {
    return rtv_heap_;
  }
  DescriptorHeapAllocator& GetDsvAllocator() {
    return dsv_heap_;
  }

  // Static: For textures (Never resets)
  DescriptorHeapAllocator& GetSrvStaticAllocator() {
    return srv_static_;
  }

  // Dynamic: For UI/Frame data (Resets per frame)
  DescriptorHeapAllocator& GetSrvDynamicAllocator(uint32_t frame_index) {
    return *srv_dynamic_[frame_index];
  }

  // Helper to get the raw global heap if needed
  ID3D12DescriptorHeap* GetGlobalSrvHeap() const {
    return global_srv_heap_.Get();
  }

 private:
  ComPtr<ID3D12DescriptorHeap> global_srv_heap_;

  DescriptorHeapAllocator rtv_heap_;
  DescriptorHeapAllocator dsv_heap_;

  // Sub-allocators pointing to global_srv_heap_
  DescriptorHeapAllocator srv_static_;
  std::vector<std::unique_ptr<DescriptorHeapAllocator>> srv_dynamic_;

  DescriptorHeapConfig config_;
};
