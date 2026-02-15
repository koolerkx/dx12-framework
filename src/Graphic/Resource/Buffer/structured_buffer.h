/**
 * @file structured_buffer.h
 * @brief The abstracted structured buffer handle resource

 * @note the buffer will stay 1-2 frame before cleanup
 * since we taking deferred clean up policy, it will auto queue to cleanup at next frame
 */
#pragma once

#include <d3d12.h>

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "Core/types.h"

namespace Graphics {

template <typename T>
class StructuredBuffer {
 public:
  StructuredBuffer() = default;

  ~StructuredBuffer() {
    if (buffer_ && mapped_data_) {
      buffer_->Unmap(0, nullptr);
      mapped_data_ = nullptr;
    }
  }

  StructuredBuffer(const StructuredBuffer&) = delete;
  StructuredBuffer& operator=(const StructuredBuffer&) = delete;

  StructuredBuffer(StructuredBuffer&& other) noexcept
      : buffer_(std::move(other.buffer_)), mapped_data_(other.mapped_data_), capacity_(other.capacity_) {
    other.mapped_data_ = nullptr;
    other.capacity_ = 0;
  }

  StructuredBuffer& operator=(StructuredBuffer&& other) noexcept {
    if (this != &other) {
      if (buffer_ && mapped_data_) {
        buffer_->Unmap(0, nullptr);
      }
      buffer_ = std::move(other.buffer_);
      mapped_data_ = other.mapped_data_;
      capacity_ = other.capacity_;
      other.mapped_data_ = nullptr;
      other.capacity_ = 0;
    }
    return *this;
  }

  bool Initialize(ID3D12Device* device, uint32_t capacity, const std::wstring& debug_name = L"StructuredBuffer") {
    if (!device || capacity == 0) return false;

    capacity_ = capacity;
    UINT64 buffer_size = static_cast<UINT64>(capacity) * sizeof(T);

    D3D12_HEAP_PROPERTIES heap_props = {};
    heap_props.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC resource_desc = {};
    resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resource_desc.Width = buffer_size;
    resource_desc.Height = 1;
    resource_desc.DepthOrArraySize = 1;
    resource_desc.MipLevels = 1;
    resource_desc.Format = DXGI_FORMAT_UNKNOWN;
    resource_desc.SampleDesc.Count = 1;
    resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    HRESULT hr = device->CreateCommittedResource(
      &heap_props, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer_));

    if (FAILED(hr)) {
      capacity_ = 0;
      return false;
    }

    buffer_->SetName(debug_name.c_str());

    D3D12_RANGE read_range = {0, 0};
    hr = buffer_->Map(0, &read_range, reinterpret_cast<void**>(&mapped_data_));

    if (FAILED(hr)) {
      buffer_.Reset();
      capacity_ = 0;
      mapped_data_ = nullptr;
      return false;
    }

    return true;
  }

  void Update(const std::vector<T>& data) {
    Update(data.data(), static_cast<uint32_t>(data.size()));
  }

  void Update(const T* data, uint32_t count) {
    if (!mapped_data_) return;
    uint32_t copy_count = (count < capacity_) ? count : capacity_;
    memcpy(mapped_data_, data, static_cast<size_t>(copy_count) * sizeof(T));
  }

  D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const {
    return buffer_ ? buffer_->GetGPUVirtualAddress() : 0;
  }

  D3D12_SHADER_RESOURCE_VIEW_DESC GetSRVDesc() const {
    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Format = DXGI_FORMAT_UNKNOWN;
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv_desc.Buffer.NumElements = capacity_;
    srv_desc.Buffer.StructureByteStride = sizeof(T);
    return srv_desc;
  }

  uint32_t GetCapacity() const {
    return capacity_;
  }
  bool IsInitialized() const {
    return buffer_ != nullptr;
  }

 private:
  ComPtr<ID3D12Resource> buffer_;
  T* mapped_data_ = nullptr;
  uint32_t capacity_ = 0;
};

}  // namespace Graphics
