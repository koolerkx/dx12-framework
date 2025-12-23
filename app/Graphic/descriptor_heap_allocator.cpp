#include "descriptor_heap_allocator.h"

#include <d3d12.h>

#include <cassert>
#include <iostream>

bool DescriptorHeapAllocator::Initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t capacity, bool shader_visible) {
  assert(device != nullptr);
  assert(capacity > 0);

  type_ = type;
  capacity_ = capacity;
  shader_visible_ = shader_visible;
  descriptor_size_ = device->GetDescriptorHandleIncrementSize(type);

  D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
  heap_desc.Type = type;
  heap_desc.NumDescriptors = capacity;
  heap_desc.NodeMask = 0;  // マルチGPU関連
  heap_desc.Flags = shader_visible_ ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

  HRESULT hr = device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&heap_));
  if (FAILED(hr)) {
    std::cerr << "DescriptorAllocator::Initialize - Failed to create descriptor heap" << std::endl;
    return false;
  }

  heap_start_cpu_ = heap_->GetCPUDescriptorHandleForHeapStart();
  if(shader_visible) {
    heap_start_gpu_ = heap_->GetGPUDescriptorHandleForHeapStart();
  }

  const wchar_t* typeName;
  switch (type) {
    case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
      typeName = L"CBV_SRV_UAV";
      break;
    case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
      typeName = L"SAMPLER";
      break;
    case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
      typeName = L"RTV";
      break;
    case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
      typeName = L"DSV";
      break;
    default:
      typeName = L"UNKNOWN";
      break;
  }

  std::wstring name = std::wstring(L"DescriptorHeap_") + typeName;
  if (shader_visible) {
    name += L"_ShaderVisible";
  }
  heap_->SetName(name.c_str());

  return true;
}

DescriptorHeapAllocator::Allocation DescriptorHeapAllocator::Allocate(uint32_t count) {
  assert(count > 0);

  if (!free_list_.empty() && count == 1) {
    // Allocate from free list
    uint32_t index = free_list_.top();
    free_list_.pop();

    Allocation allocation;
    allocation.cpu = GetCpuHandle(index);
    allocation.gpu = shader_visible_ ? GetGpuHandle(index) : D3D12_GPU_DESCRIPTOR_HANDLE{0};
    allocation.index = index;
    allocation.count = count;

    return allocation;
  }

  if (allocated_ + count > capacity_) {
    std::cerr << "DescriptorAllocator::Allocate - Out of descriptors! "
              << "Requested: " << count << ", Available: " << (capacity_ - allocated_) << std::endl;
  }

  // Allocate from end
  // TOOD: Review logic
  uint32_t index = allocated_;
  allocated_ += count;

  Allocation allocation;
  allocation.cpu = GetCpuHandle(index);
  allocation.gpu = shader_visible_ ? GetGpuHandle(index) : D3D12_GPU_DESCRIPTOR_HANDLE{0};
  allocation.index = index;
  allocation.count = count;

  return allocation;
}

void DescriptorHeapAllocator::Free(const Allocation& allocation) {
  if (!allocation.IsValid()) {
    return;
  }

  if (allocation.count == 1) {
    free_list_.push(allocation.index);
  } else {
    // TOOD: multi-descriptor free
  }
}

void DescriptorHeapAllocator::Reset() {
  allocated_ = 0;
  while (!free_list_.empty()) {
    free_list_.pop();
  }
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapAllocator::GetCpuHandle(uint32_t index) const {
  D3D12_CPU_DESCRIPTOR_HANDLE handle = heap_start_cpu_;
  handle.ptr += static_cast<SIZE_T>(index) * descriptor_size_;
  return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeapAllocator::GetGpuHandle(uint32_t index) const {
  if (!shader_visible_) {
    return D3D12_GPU_DESCRIPTOR_HANDLE{0};
  }

  D3D12_GPU_DESCRIPTOR_HANDLE handle = heap_start_gpu_;
  handle.ptr += static_cast<UINT64>(index) * descriptor_size_;
  return handle;
}
