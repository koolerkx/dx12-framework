#pragma once
#include <d3d12.h>
#include <wrl/client.h>

#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

class DynamicUploadBuffer {
 public:
  struct Allocation {
    void* cpu_ptr;
    D3D12_GPU_VIRTUAL_ADDRESS gpu_ptr;
  };

  DynamicUploadBuffer() = default;

  ~DynamicUploadBuffer() {
    for (auto& page : pages_) {
      if (page.resource && page.cpu_ptr) {
        page.resource->Unmap(0, nullptr);
      }
    }
    pages_.clear();
  }

  bool Initialize(ID3D12Device* device, size_t page_size, std::wstring name) {
    name_ = name;
    device_ = device;
    page_size_ = page_size;

    return CreateNewPage();
  }

  void Reset() {
    current_page_index_ = 0;
    cursor_ = 0;
  }

  template <typename T>
  Allocation Allocate() {
    return Allocate(sizeof(T));
  }

  Allocation Allocate(size_t size_in_bytes) {
    static constexpr size_t D3D12_CONSTANT_BUFFER_ALIGNMENT = 256;
    size_t aligned_size = (size_in_bytes + D3D12_CONSTANT_BUFFER_ALIGNMENT - 1) & ~(D3D12_CONSTANT_BUFFER_ALIGNMENT - 1);

    if (aligned_size > page_size_) {
      throw std::runtime_error("DynamicUploadBuffer: Single allocation too large for page size.");
    }

    // check need to switch page
    if (cursor_ + aligned_size > page_size_) {
      current_page_index_++;
      cursor_ = 0;

      // create new page
      if (current_page_index_ >= pages_.size()) {
        if (!CreateNewPage()) {
          throw std::runtime_error("DynamicUploadBuffer: Failed to create new page.");
        }
      }
    }

    auto& current_page = pages_[current_page_index_];

    Allocation alloc;
    alloc.cpu_ptr = current_page.cpu_ptr + cursor_;
    alloc.gpu_ptr = current_page.gpu_ptr + cursor_;

    cursor_ += aligned_size;
    return alloc;
  }

 private:
  std::wstring name_;

  struct Page {
    Microsoft::WRL::ComPtr<ID3D12Resource> resource;
    uint8_t* cpu_ptr = nullptr;
    D3D12_GPU_VIRTUAL_ADDRESS gpu_ptr = 0;
  };

  ID3D12Device* device_ = nullptr;
  std::vector<Page> pages_;
  size_t page_size_ = 0;
  size_t current_page_index_ = 0;
  size_t cursor_ = 0;

  bool CreateNewPage() {
    Page newPage;

    D3D12_HEAP_PROPERTIES heapProps = {D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1, 1};
    D3D12_RESOURCE_DESC desc = {.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
      .Alignment = 0,
      .Width = page_size_,
      .Height = 1,
      .DepthOrArraySize = 1,
      .MipLevels = 1,
      .Format = DXGI_FORMAT_UNKNOWN,
      .SampleDesc = {1, 0},
      .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
      .Flags = D3D12_RESOURCE_FLAG_NONE};

    if (FAILED(device_->CreateCommittedResource(
          &heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&newPage.resource)))) {
      return false;
    }

    std::wstring debugName = name_ + L"_Page" + std::to_wstring(pages_.size());
    newPage.resource->SetName(debugName.c_str());

    if (FAILED(newPage.resource->Map(0, nullptr, reinterpret_cast<void**>(&newPage.cpu_ptr)))) {
      return false;
    }

    newPage.gpu_ptr = newPage.resource->GetGPUVirtualAddress();
    pages_.push_back(newPage);
    return true;
  }
};
