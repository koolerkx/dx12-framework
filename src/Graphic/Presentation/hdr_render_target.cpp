#include "hdr_render_target.h"

#include <d3d12.h>

#include "d3dx12.h"
#include "Framework/Logging/logger.h"

bool HdrRenderTarget::Initialize(ID3D12Device* device, UINT width, UINT height, DescriptorHeapManager& descriptor_manager) {
  width_ = width;
  height_ = height;

  // Clear color (black, opaque)
  D3D12_CLEAR_VALUE clear_value = {};
  clear_value.Format = FORMAT;
  clear_value.Color[0] = 0.0f;
  clear_value.Color[1] = 0.0f;
  clear_value.Color[2] = 0.0f;
  clear_value.Color[3] = 1.0f;

  CD3DX12_RESOURCE_DESC resource_desc =
    CD3DX12_RESOURCE_DESC::Tex2D(FORMAT, width, height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

  CD3DX12_HEAP_PROPERTIES heap_props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

  ComPtr<ID3D12Resource> resource;
  HRESULT hr = device->CreateCommittedResource(&heap_props,
    D3D12_HEAP_FLAG_NONE,
    &resource_desc,
    D3D12_RESOURCE_STATE_RENDER_TARGET,
    &clear_value,
    IID_PPV_ARGS(resource.GetAddressOf()));

  if (FAILED(hr)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[HdrRenderTarget] Failed to create resource");
    return false;
  }

  SetResource(resource, D3D12_RESOURCE_STATE_RENDER_TARGET);

  // Allocate RTV
  rtv_allocation_ = descriptor_manager.GetRtvAllocator().Allocate(1);
  if (!rtv_allocation_.IsValid()) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[HdrRenderTarget] Failed to allocate RTV");
    return false;
  }

  device->CreateRenderTargetView(resource.Get(), nullptr, rtv_allocation_.cpu);

  // Allocate SRV (for tone mapping)
  auto& srv_allocator = descriptor_manager.GetSrvStaticAllocator();
  DescriptorHeapAllocator::Allocation srv_allocation = srv_allocator.Allocate(1);
  if (!srv_allocation.IsValid()) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[HdrRenderTarget] Failed to allocate SRV");
    return false;
  }
  srv_index_ = srv_allocation.index;

  D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
  srv_desc.Format = FORMAT;
  srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srv_desc.Texture2D.MipLevels = 1;
  srv_desc.Texture2D.MostDetailedMip = 0;
  srv_desc.Texture2D.PlaneSlice = 0;
  srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;

  device->CreateShaderResourceView(resource.Get(), &srv_desc, srv_allocation.cpu);

  SetDebugName("HdrRenderTarget");
  return true;
}

void HdrRenderTarget::Clear(ID3D12GraphicsCommandList* command_list) {
  if (!IsValid()) return;
  constexpr float clear_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
  command_list->ClearRenderTargetView(rtv_allocation_.cpu, clear_color, 0, nullptr);
}

// RTV uses frame allocator (auto-recycled), but SRV uses static allocator (must free)
void HdrRenderTarget::SafeRelease(DescriptorHeapManager& descriptor_manager) {
  // Release resource
  resource_.Reset();

  // Release SRV from static allocator (critical - static allocators don't auto-recycle)
  if (srv_index_ != 0) {
    descriptor_manager.GetSrvStaticAllocator().FreeImmediate(srv_index_, 1);
    srv_index_ = 0;
  }

  // RTV uses frame allocator which auto-recycles each frame,
  // but we invalidate the allocation handle for safety
  rtv_allocation_ = DescriptorHeapAllocator::Allocation{};
}
