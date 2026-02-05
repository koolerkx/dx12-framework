#pragma once

#include <dxgiformat.h>

#include <array>
#include <cstdint>

#include "Descriptor/descriptor_heap_allocator.h"
#include "Descriptor/descriptor_heap_manager.h"
#include "Device/gpu_resource.h"

class RenderTexture : public GpuResource {
 public:
  RenderTexture(DXGI_FORMAT format, std::array<float, 4> clear_color = {0, 0, 0, 1});
  ~RenderTexture() = default;

  bool Initialize(ID3D12Device* device, uint32_t width, uint32_t height, DescriptorHeapManager& heap_mgr);
  void SafeRelease(DescriptorHeapManager& heap_mgr);
  void Clear(ID3D12GraphicsCommandList* cmd);

  D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const {
    return rtv_allocation_.cpu;
  }
  uint32_t GetSrvIndex() const {
    return srv_index_;
  }
  uint32_t GetWidth() const {
    return width_;
  }
  uint32_t GetHeight() const {
    return height_;
  }
  DXGI_FORMAT GetFormat() const {
    return format_;
  }

  void ResetFrameState() {
    cleared_this_frame_ = false;
  }
  bool NeedsClear() const {
    return !cleared_this_frame_;
  }
  void MarkCleared() {
    cleared_this_frame_ = true;
  }

  bool IsValid() const override {
    return GpuResource::IsValid() && rtv_allocation_.IsValid();
  }

 private:
  DXGI_FORMAT format_;
  std::array<float, 4> clear_color_;
  DescriptorHeapAllocator::Allocation rtv_allocation_;
  uint32_t srv_index_ = 0;
  uint32_t width_ = 0;
  uint32_t height_ = 0;
  bool cleared_this_frame_ = false;
};
