#include "depth_buffer.h"

#include <d3d12.h>
#include <winerror.h>

#include <cassert>

#include "Core/types.h"
#include "Framework/Logging/logger.h"
#include "d3dx12.h"


bool DepthBuffer::Initialize(ID3D12Device* device, UINT width, UINT height, DescriptorHeapManager& descriptor_manager, DXGI_FORMAT format) {
  assert(device != nullptr);

  width_ = width;
  height_ = height;
  format_ = format;

  DXGI_FORMAT typeless_format = DXGI_FORMAT_R32_TYPELESS;

  D3D12_CLEAR_VALUE clear_value = {};
  clear_value.Format = DXGI_FORMAT_D32_FLOAT;
  clear_value.DepthStencil.Depth = 1.0f;
  clear_value.DepthStencil.Stencil = 0;

  CD3DX12_RESOURCE_DESC resource_desc =
    CD3DX12_RESOURCE_DESC::Tex2D(typeless_format, width, height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
  CD3DX12_HEAP_PROPERTIES heap_props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

  ComPtr<ID3D12Resource> resource;
  HRESULT hr = device->CreateCommittedResource(&heap_props,
    D3D12_HEAP_FLAG_NONE,
    &resource_desc,
    D3D12_RESOURCE_STATE_DEPTH_WRITE,
    &clear_value,
    IID_PPV_ARGS(resource.GetAddressOf()));

  if (FAILED(hr)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[DepthBuffer] Failed to create depth stencil resource");
    return false;
  }

  SetResource(resource, D3D12_RESOURCE_STATE_DEPTH_WRITE);

  dsv_allocation_ = descriptor_manager.GetDsvAllocator().Allocate(1);
  if (!dsv_allocation_.IsValid()) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[DepthBuffer] Failed to allocate DSV");
    return false;
  }

  D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
  dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
  dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
  dsv_desc.Flags = D3D12_DSV_FLAG_NONE;
  dsv_desc.Texture2D.MipSlice = 0;

  device->CreateDepthStencilView(resource.Get(), &dsv_desc, dsv_allocation_.cpu);

  auto& srv_allocator = descriptor_manager.GetSrvStaticAllocator();
  auto srv_allocation = srv_allocator.Allocate(1);
  if (!srv_allocation.IsValid()) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[DepthBuffer] Failed to allocate SRV");
    return false;
  }
  srv_index_ = srv_allocation.index;

  D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
  srv_desc.Format = DXGI_FORMAT_R32_FLOAT;
  srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srv_desc.Texture2D.MipLevels = 1;
  srv_desc.Texture2D.MostDetailedMip = 0;
  srv_desc.Texture2D.PlaneSlice = 0;
  srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;

  device->CreateShaderResourceView(resource.Get(), &srv_desc, srv_allocation.cpu);

  SetDebugName("DepthBuffer");
  return true;
}

void DepthBuffer::Clear(ID3D12GraphicsCommandList* command_list, float depth, UINT8 stencil) {
  assert(command_list != nullptr);
  if (!IsValid()) return;

  command_list->ClearDepthStencilView(dsv_allocation_.cpu, D3D12_CLEAR_FLAG_DEPTH, depth, stencil, 0, nullptr);
  cleared_this_frame_ = true;
}

void DepthBuffer::SafeRelease(DescriptorHeapManager& heap_mgr) {
  if (dsv_allocation_.IsValid()) {
    heap_mgr.GetDsvAllocator().FreeImmediate(dsv_allocation_.index, 1);
    dsv_allocation_ = {};
  }

  if (srv_index_ != 0) {
    heap_mgr.GetSrvStaticAllocator().FreeImmediate(srv_index_, 1);
    srv_index_ = 0;
  }

  resource_.Reset();
}
