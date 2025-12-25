#include "depth_buffer.h"

#include <d3d12.h>
#include <winerror.h>

#include <cassert>
#include <iostream>

#include "d3dx12.h"
#include "Core/types.h"

bool DepthBuffer::Initialize(ID3D12Device* device, UINT width, UINT height, DescriptorHeapManager& descriptor_manager, DXGI_FORMAT format) {
  assert(device != nullptr);

  width_ = width;
  height_ = height;
  format_ = format;

  D3D12_CLEAR_VALUE clear_value = {};
  clear_value.Format = format_;
  clear_value.DepthStencil.Depth = 1.0f;
  clear_value.DepthStencil.Stencil = 0;

  CD3DX12_RESOURCE_DESC resource_desc = CD3DX12_RESOURCE_DESC::Tex2D(
    format, width, height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);
  CD3DX12_HEAP_PROPERTIES heap_props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

  ComPtr<ID3D12Resource> resource;
  HRESULT hr = device->CreateCommittedResource(&heap_props,
    D3D12_HEAP_FLAG_NONE,
    &resource_desc,
    D3D12_RESOURCE_STATE_DEPTH_WRITE,
    &clear_value,
    IID_PPV_ARGS(resource.GetAddressOf()));

  if (FAILED(hr)) {
    std::cerr << "[DepthBuffer] Failed to create depth stencil resource" << std::endl;
    return false;
  }

  SetResource(resource, D3D12_RESOURCE_STATE_DEPTH_WRITE);

  dsv_allocation_ = descriptor_manager.GetDsvAllocator().Allocate(1);
  if (!dsv_allocation_.IsValid()) {
    std::cerr << "[DepthBuffer] Failed to allocate DSV" << std::endl;
    return false;
  }

  D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
  dsvDesc.Format = format;
  dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
  dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
  dsvDesc.Texture2D.MipSlice = 0;

  device->CreateDepthStencilView(resource.Get(), &dsvDesc, dsv_allocation_.cpu);

  SetDebugName("DepthBuffer");
  return true;
}

void DepthBuffer::Clear(ID3D12GraphicsCommandList* command_list, float depth, UINT8 stencil) {
  assert(command_list != nullptr);
  if (!IsValid()) return;

  command_list->ClearDepthStencilView(dsv_allocation_.cpu, D3D12_CLEAR_FLAG_DEPTH, depth, stencil, 0, nullptr);
}
