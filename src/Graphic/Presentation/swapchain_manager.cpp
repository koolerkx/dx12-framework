#include "swapchain_manager.h"

#include <d3d12.h>
#include <dxgi.h>
#include <dxgiformat.h>
#include <winerror.h>
#include <winnt.h>

#include <cassert>
#include <string>

#include "Framework/Logging/logger.h"

bool SwapChainManager::Initialize(ID3D12Device* device,
  IDXGIFactory6* factory,
  ID3D12CommandQueue* command_queue,
  HWND hwnd,
  UINT width,
  UINT height,
  DescriptorHeapManager& descriptor_manager,
  bool allow_tearing) {
  assert(device != nullptr);
  assert(factory != nullptr);
  assert(command_queue != nullptr);
  assert(hwnd != nullptr);

  device_ = device;
  width_ = width;
  height_ = height;

  DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
  swap_chain_desc.Width = width_;
  swap_chain_desc.Height = height_;
  swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swap_chain_desc.Stereo = false;
  swap_chain_desc.SampleDesc.Count = 1;
  swap_chain_desc.SampleDesc.Quality = 0;
  swap_chain_desc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
  swap_chain_desc.BufferCount = BUFFER_COUNT;
  swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
  swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
  swap_chain_flags_ = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | (allow_tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);
  swap_chain_desc.Flags = swap_chain_flags_;

  ComPtr<IDXGISwapChain1> swap_chain1;
  HRESULT hr = factory->CreateSwapChainForHwnd(command_queue, hwnd, &swap_chain_desc, nullptr, nullptr, swap_chain1.GetAddressOf());

  if (FAILED(hr) || swap_chain1 == nullptr) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[SwapChainManager] Failed to create swap chain");
    return false;
  }

  swap_chain1.As(&swap_chain_);

  if (FAILED(hr) || swap_chain_ == nullptr) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[SwapChainManager] Failed to create swap chain");
    return false;
  }

  if (!CreateBackBufferViews(descriptor_manager)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[SwapChainManager] Failed to create back buffer views");
  }

  return true;
}

void SwapChainManager::SafeRelease() {
  bool expected = false;
  if (!is_released_.compare_exchange_strong(expected, true)) {
    return;
  }

  ReleaseBackBuffers();

  if (swap_chain_) {
    swap_chain_.Reset();
  }

  device_ = nullptr;
}

bool SwapChainManager::CreateBackBufferViews(DescriptorHeapManager& descriptor_manager) {
  for (UINT i = 0; i < BUFFER_COUNT; ++i) {
    HRESULT hr = swap_chain_->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(back_buffers_[i].GetAddressOf()));
    if (FAILED(hr)) {
      Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "Failed to get back buffer {}", i);
      return false;
    }

    back_buffer_rtvs_[i] = descriptor_manager.GetRtvAllocator().Allocate(1);
    if (!back_buffer_rtvs_[i].IsValid()) {
      Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "Failed to allocate RTV for back buffer {}", i);
      return false;
    }

    device_->CreateRenderTargetView(back_buffers_[i].Get(), nullptr, back_buffer_rtvs_[i].cpu);

    std::wstring name = L"BackBuffer_" + std::to_wstring(i);
    back_buffers_[i]->SetName(name.c_str());
  }

  return true;
}

void SwapChainManager::TransitionToRenderTarget(ID3D12GraphicsCommandList* command_list) {
  UINT index = GetCurrentBackBufferIndex();

  D3D12_RESOURCE_BARRIER barrier = {};
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource = back_buffers_[index].Get();
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

  command_list->ResourceBarrier(1, &barrier);
}

void SwapChainManager::TransitionToPresent(ID3D12GraphicsCommandList* command_list) {
  UINT index = GetCurrentBackBufferIndex();

  D3D12_RESOURCE_BARRIER barrier = {};
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource = back_buffers_[index].Get();
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

  command_list->ResourceBarrier(1, &barrier);
}

void SwapChainManager::Present(UINT syncInterval, UINT flags) {
  if (swap_chain_) {
    swap_chain_->Present(syncInterval, flags);
  }
}

void SwapChainManager::ReleaseBackBuffers() {
  for (auto& buffer : back_buffers_) {
    buffer.Reset();
  }
}

bool SwapChainManager::Resize(UINT width, UINT height, DescriptorHeapManager& descriptor_manager) {
  assert(swap_chain_ != nullptr);

  ReleaseBackBuffers();
  HRESULT hr = swap_chain_->ResizeBuffers(BUFFER_COUNT, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, swap_chain_flags_);

  if (FAILED(hr)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[SwapChainManager] Failed to resize swap chain");
    return false;
  }

  width_ = width;
  height_ = height;

  return CreateBackBufferViews(descriptor_manager);
}
