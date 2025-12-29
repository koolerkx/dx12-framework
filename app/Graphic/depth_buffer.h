#pragma once

#include <dxgiformat.h>

#include "descriptor_heap_allocator.h"
#include "descriptor_heap_manager.h"
#include "gpu_resource.h"

class DepthBuffer : public GpuResource {
 public:
  DepthBuffer() = default;
  ~DepthBuffer() = default;

  DepthBuffer(const DepthBuffer&) = delete;
  DepthBuffer& operator=(const DepthBuffer&) = delete;

  bool Initialize(
    ID3D12Device* device, UINT width, UINT height, DescriptorHeapManager& descriptor_manager, DXGI_FORMAT format = DXGI_FORMAT_D32_FLOAT);

  D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const {
    return dsv_allocation_.cpu;
  }

  void Clear(ID3D12GraphicsCommandList* command_list, float depth = 1.0f, UINT8 stencil = 0);

  void SafeRelease();

  UINT GetWidth() const {
    return width_;
  }
  UINT GetHeight() const {
    return height_;
  }

  bool IsValid() const override {
    return GpuResource::IsValid() && dsv_allocation_.IsValid();
  }

 private:
  DescriptorHeapAllocator::Allocation dsv_allocation_;
  UINT width_ = 0;
  UINT height_ = 0;
  DXGI_FORMAT format_ = DXGI_FORMAT_D32_FLOAT;
};
