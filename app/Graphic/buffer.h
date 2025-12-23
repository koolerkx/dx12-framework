#pragma once

#include <d3d12.h>

#include <stdexcept>

#include "types.h"

class Buffer {
 protected:
  ComPtr<ID3D12Resource> CreateUploadBuffer(ID3D12Device* device, size_t bufferSize, const void* initialData = nullptr) {
    // Heap properties
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    // Resource description
    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = bufferSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    ComPtr<ID3D12Resource> buffer;
    HRESULT hr = device->CreateCommittedResource(
      &heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer));

    if (FAILED(hr)) {
      throw std::runtime_error("Failed to create buffer");
    }

    // Upload initial data if provided
    if (initialData) {
      void* mappedData = nullptr;
      buffer->Map(0, nullptr, &mappedData);
      memcpy(mappedData, initialData, bufferSize);
      buffer->Unmap(0, nullptr);
    }

    return buffer;
  }
};

class VertexBuffer : Buffer {
 public:
  template <typename VertexType>
  bool Create(ID3D12Device* device, const VertexType* vertices, size_t count) {
    size_t bufferSize = sizeof(VertexType) * count;

    resource_ = CreateUploadBuffer(device, bufferSize, vertices);

    view_.BufferLocation = resource_->GetGPUVirtualAddress();
    view_.SizeInBytes = static_cast<UINT>(bufferSize);
    view_.StrideInBytes = sizeof(VertexType);

    return resource_ != nullptr;
  }

  D3D12_VERTEX_BUFFER_VIEW GetView() const {
    return view_;
  }
  ID3D12Resource* GetResource() const {
    return resource_.Get();
  }

 private:
  ComPtr<ID3D12Resource> resource_;
  D3D12_VERTEX_BUFFER_VIEW view_;
};

class IndexBuffer : Buffer {
 public:
  template <typename IndexType>
  bool Create(ID3D12Device* device, const IndexType* indices, size_t count) {
    static_assert(std::is_same_v<IndexType, uint16_t> || std::is_same_v<IndexType, uint32_t>, "Index type must be uint16_t or uint32_t");

    size_t bufferSize = sizeof(IndexType) * count;
    resource_ = CreateUploadBuffer(device, bufferSize, indices);

    view_.BufferLocation = resource_->GetGPUVirtualAddress();
    view_.SizeInBytes = static_cast<UINT>(bufferSize);
    view_.Format = std::is_same_v<IndexType, uint16_t> ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

    return resource_ != nullptr;
  }

  D3D12_INDEX_BUFFER_VIEW GetView() const {
    return view_;
  }
  ID3D12Resource* GetResource() const {
    return resource_.Get();
  }

 private:
  ComPtr<ID3D12Resource> resource_;
  D3D12_INDEX_BUFFER_VIEW view_;
};
