#pragma once

#include <d3d12.h>

#include <stdexcept>

#include "Core/types.h"

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

template <typename T>
class ConstantBuffer : Buffer {
 public:
  ConstantBuffer() : mappedData_(nullptr), bufferSize_(0) {
  }

  ~ConstantBuffer() {
    if (resource_ && mappedData_) {
      resource_->Unmap(0, nullptr);
      mappedData_ = nullptr;
    }
  }

  ConstantBuffer(const ConstantBuffer&) = delete;
  ConstantBuffer& operator=(const ConstantBuffer&) = delete;

  ConstantBuffer(ConstantBuffer&& other) noexcept
      : resource_(std::move(other.resource_)), mappedData_(other.mappedData_), bufferSize_(other.bufferSize_) {
    other.mappedData_ = nullptr;
    other.bufferSize_ = 0;
  }

  ConstantBuffer& operator=(ConstantBuffer&& other) noexcept {
    if (this != &other) {
      if (resource_ && mappedData_) {
        resource_->Unmap(0, nullptr);
      }
      resource_ = std::move(other.resource_);
      mappedData_ = other.mappedData_;
      bufferSize_ = other.bufferSize_;
      other.mappedData_ = nullptr;
      other.bufferSize_ = 0;
    }
    return *this;
  }

  bool Create(ID3D12Device* device) {
    bufferSize_ = (sizeof(T) + 255) & ~255;  // 256 byte alignment

    resource_ = CreateUploadBuffer(device, bufferSize_, nullptr);

    if (!resource_) {
      return false;
    }

    HRESULT hr = resource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedData_));
    return SUCCEEDED(hr);
  }

  void Update(const T& data) {
    if (mappedData_) {
      memcpy(mappedData_, &data, sizeof(T));
    }
  }

  D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const {
    return resource_ ? resource_->GetGPUVirtualAddress() : 0;
  }

  ID3D12Resource* GetResource() const {
    return resource_.Get();
  }

 private:
  ComPtr<ID3D12Resource> resource_;
  T* mappedData_;
  size_t bufferSize_;
};
