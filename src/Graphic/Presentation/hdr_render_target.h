#pragma once
#include <dxgiformat.h>
#include "Descriptor/descriptor_heap_allocator.h"
#include "Descriptor/descriptor_heap_manager.h"
#include "Device/gpu_resource.h"

class DescriptorHeapManager;

class HdrRenderTarget : public GpuResource {
 public:
  HdrRenderTarget() = default;
  ~HdrRenderTarget() = default;

  bool Initialize(ID3D12Device* device, UINT width, UINT height,
                  DescriptorHeapManager& descriptor_manager);

  D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const { return rtv_allocation_.cpu; }
  uint32_t GetSrvIndex() const { return srv_index_; }

  void Clear(ID3D12GraphicsCommandList* command_list);

  // CRITICAL: Must properly release descriptor allocations to avoid leaks
  void SafeRelease(DescriptorHeapManager& descriptor_manager);

  bool IsValid() const override {
    return GpuResource::IsValid() && rtv_allocation_.IsValid();
  }

 private:
  DescriptorHeapAllocator::Allocation rtv_allocation_;
  uint32_t srv_index_ = 0;
  UINT width_ = 0;
  UINT height_ = 0;
  static constexpr DXGI_FORMAT FORMAT = DXGI_FORMAT_R16G16B16A16_FLOAT;
};
