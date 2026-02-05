#include "render_texture.h"

#include <d3d12.h>

#include "Framework/Logging/logger.h"
#include "d3dx12.h"

RenderTexture::RenderTexture(DXGI_FORMAT format, std::array<float, 4> clear_color) : format_(format), clear_color_(clear_color) {
}

bool RenderTexture::Initialize(ID3D12Device* device, uint32_t width, uint32_t height, DescriptorHeapManager& heap_mgr) {
  width_ = width;
  height_ = height;

  D3D12_CLEAR_VALUE clear_value = {};
  clear_value.Format = format_;
  clear_value.Color[0] = clear_color_[0];
  clear_value.Color[1] = clear_color_[1];
  clear_value.Color[2] = clear_color_[2];
  clear_value.Color[3] = clear_color_[3];

  CD3DX12_RESOURCE_DESC resource_desc =
    CD3DX12_RESOURCE_DESC::Tex2D(format_, width, height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

  CD3DX12_HEAP_PROPERTIES heap_props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

  ComPtr<ID3D12Resource> resource;
  HRESULT hr = device->CreateCommittedResource(&heap_props,
    D3D12_HEAP_FLAG_NONE,
    &resource_desc,
    D3D12_RESOURCE_STATE_RENDER_TARGET,
    &clear_value,
    IID_PPV_ARGS(resource.GetAddressOf()));

  if (FAILED(hr)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[RenderTexture] Failed to create resource");
    return false;
  }

  SetResource(resource, D3D12_RESOURCE_STATE_RENDER_TARGET);

  rtv_allocation_ = heap_mgr.GetRtvAllocator().Allocate(1);
  if (!rtv_allocation_.IsValid()) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[RenderTexture] Failed to allocate RTV");
    return false;
  }

  device->CreateRenderTargetView(resource.Get(), nullptr, rtv_allocation_.cpu);

  auto& srv_allocator = heap_mgr.GetSrvStaticAllocator();
  DescriptorHeapAllocator::Allocation srv_allocation = srv_allocator.Allocate(1);
  if (!srv_allocation.IsValid()) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[RenderTexture] Failed to allocate SRV");
    return false;
  }
  srv_index_ = srv_allocation.index;

  D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
  srv_desc.Format = format_;
  srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srv_desc.Texture2D.MipLevels = 1;
  srv_desc.Texture2D.MostDetailedMip = 0;
  srv_desc.Texture2D.PlaneSlice = 0;
  srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;

  device->CreateShaderResourceView(resource.Get(), &srv_desc, srv_allocation.cpu);

  SetDebugName("RenderTexture");
  return true;
}

void RenderTexture::Clear(ID3D12GraphicsCommandList* cmd) {
  if (!IsValid()) return;
  cmd->ClearRenderTargetView(rtv_allocation_.cpu, clear_color_.data(), 0, nullptr);
  cleared_this_frame_ = true;
}

void RenderTexture::SafeRelease(DescriptorHeapManager& heap_mgr) {
  resource_.Reset();

  if (srv_index_ != 0) {
    heap_mgr.GetSrvStaticAllocator().FreeImmediate(srv_index_, 1);
    srv_index_ = 0;
  }

  rtv_allocation_ = DescriptorHeapAllocator::Allocation{};
}
