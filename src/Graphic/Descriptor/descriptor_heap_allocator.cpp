#include "descriptor_heap_allocator.h"

#include <algorithm>
#include <cassert>

#include "Framework/Logging/logger.h"

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
  if (shader_visible) {
    start_gpu_ = heap_->GetGPUDescriptorHandleForHeapStart();
  } else {
    start_gpu_.ptr = 0;
  }

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
  D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
  bool is_shader_visible = (desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) != 0;

  if (is_shader_visible) {
    auto gpu_start = heap->GetGPUDescriptorHandleForHeapStart();
    start_gpu_.ptr = gpu_start.ptr + static_cast<UINT64>(offset) * descriptor_size;
  } else {
    start_gpu_.ptr = 0;
  }
}

DescriptorHeapAllocator::Allocation DescriptorHeapAllocator::Allocate(uint32_t count) {
  std::lock_guard<std::mutex> lock(alloc_mutex_);

  for (auto it = free_list_.begin(); it != free_list_.end(); ++it) {
    if (it->size >= count) {
      Allocation alloc;
      alloc.index = it->offset;
      alloc.cpu = GetCpuHandle(it->offset);
      alloc.gpu = GetGpuHandle(it->offset);

      it->offset += count;
      it->size -= count;
      if (it->size == 0) free_list_.erase(it);
      return alloc;
    }
  }

  if (allocated_ + count > capacity_) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Resource, Logger::Here(), "Descriptor Heap Overflow!");
    return {};
  }

  Allocation alloc;
  alloc.index = allocated_;  // Relative index
  alloc.cpu = GetCpuHandle(allocated_);
  alloc.gpu = GetGpuHandle(allocated_);

  allocated_ += count;
  return alloc;
}

void DescriptorHeapAllocator::FreeImmediate(uint32_t offset, uint32_t count) {
  std::lock_guard<std::mutex> lock(alloc_mutex_);
  AddToFreeList(offset, count);
}

void DescriptorHeapAllocator::CoalesceFreeBlocks() {
  std::lock_guard<std::mutex> lock(alloc_mutex_);

  if (free_list_.empty()) return;

  // Sort by offset
  std::sort(free_list_.begin(), free_list_.end());

  // Merge adjacent blocks
  std::vector<FreeBlock> merged;
  merged.reserve(free_list_.size());
  merged.push_back(free_list_[0]);

  for (size_t i = 1; i < free_list_.size(); ++i) {
    auto& last = merged.back();
    auto& current = free_list_[i];

    // Check if adjacent
    if (last.offset + last.size == current.offset) {
      last.size += current.size;
    } else {
      merged.push_back(current);
    }
  }

  free_list_ = std::move(merged);
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

void DescriptorHeapAllocator::AddToFreeList(uint32_t offset, uint32_t count) {
  FreeBlock block = {offset, count};

  // Insert in sorted order
  auto it = std::lower_bound(free_list_.begin(), free_list_.end(), block);
  free_list_.insert(it, block);
}
