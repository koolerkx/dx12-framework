#pragma once

#include <string>

#include "d3d12.h"
#include "Core/types.h"

class GpuResource {
 public:
  GpuResource() = default;
  virtual ~GpuResource() = default;

  GpuResource(const GpuResource&) = delete;
  GpuResource& operator=(const GpuResource&) = delete;

  void TransitionTo(
    ID3D12GraphicsCommandList* command_list, D3D12_RESOURCE_STATES new_state, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

  D3D12_RESOURCE_STATES GetCurrentState() const {
    return current_state_;
  };

  ID3D12Resource* GetResource() const {
    return resource_.Get();
  };

  D3D12_RESOURCE_DESC GetDesc() const {
    return resource_ ? resource_->GetDesc() : D3D12_RESOURCE_DESC{};
  }

  void SetDebugName(const std::wstring& name);
  void SetDebugName(const std::string& name);

  virtual bool IsValid() const {
    return resource_ != nullptr;
  }

 protected:
  ComPtr<ID3D12Resource> resource_ = nullptr;
  D3D12_RESOURCE_STATES current_state_ = D3D12_RESOURCE_STATE_COMMON;
  std::string debug_name;

  void SetResource(ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES initial_state) {
    resource_ = resource;
    current_state_ = initial_state;
  }
};
