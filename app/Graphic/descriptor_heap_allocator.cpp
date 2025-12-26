#include "descriptor_heap_allocator.h"

#include <cassert>
#include <iostream>

bool DescriptorHeapAllocator::Initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t capacity, bool shader_visible) {
  assert(device != nullptr);

  capacity_ = capacity;
  descriptor_size_ = device->GetDescriptorHandleIncrementSize(type);

  D3D12_DESCRIPTOR_HEAP_DESC desc = {};
  desc.Type = type;
  desc.NumDescriptors = capacity;
  desc.Flags = shader_visible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

  if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap_)))) return false;

  external_heap_ = heap_.Get();
  start_cpu_ = heap_->GetCPUDescriptorHandleForHeapStart();
  if (shader_visible) start_gpu_ = heap_->GetGPUDescriptorHandleForHeapStart();

  return true;
}

void DescriptorHeapAllocator::InitializeSubAllocation(
  ID3D12DescriptorHeap* heap, uint32_t offset, uint32_t capacity, uint32_t descriptor_size) {
  external_heap_ = heap;
  capacity_ = capacity;
  descriptor_size_ = descriptor_size;
  allocated_ = 0;

  // Calculate start based on parent start + offset
  auto cpu_start = heap->GetCPUDescriptorHandleForHeapStart();
  start_cpu_.ptr = cpu_start.ptr + static_cast<SIZE_T>(offset) * descriptor_size;

  // GPU handle might be null if not shader visible, but we assume SRV heap is visible
  auto gpu_start = heap->GetGPUDescriptorHandleForHeapStart();
  if (gpu_start.ptr != 0) {
    start_gpu_.ptr = gpu_start.ptr + static_cast<UINT64>(offset) * descriptor_size;
  }
}

DescriptorHeapAllocator::Allocation DescriptorHeapAllocator::Allocate(uint32_t count) {
  std::lock_guard<std::mutex> lock(alloc_mutex_);

  if (allocated_ + count > capacity_) {
    std::cerr << "Descriptor Heap Overflow!" << std::endl;
    return {};
  }

  Allocation alloc;
  alloc.index = allocated_;  // Relative index
  alloc.cpu = GetCpuHandle(allocated_);
  alloc.gpu = GetGpuHandle(allocated_);

  allocated_ += count;
  return alloc;
}

void DescriptorHeapAllocator::Reset() {
  std::lock_guard<std::mutex> lock(alloc_mutex_);
  allocated_ = 0;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapAllocator::GetCpuHandle(uint32_t offset_index) const {
  D3D12_CPU_DESCRIPTOR_HANDLE h = start_cpu_;
  h.ptr += static_cast<SIZE_T>(offset_index) * descriptor_size_;
  return h;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeapAllocator::GetGpuHandle(uint32_t offset_index) const {
  D3D12_GPU_DESCRIPTOR_HANDLE h = start_gpu_;
  if (h.ptr != 0) h.ptr += static_cast<UINT64>(offset_index) * descriptor_size_;
  return h;
}
