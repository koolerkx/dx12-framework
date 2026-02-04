/**
 * @file presentation_context.cpp
 * @brief Manages the swap chain, depth and HDR render targets, viewport/scissor,
 * and the frame presentation lifecycle, including GPU synchronization and resize handling.
 */
#include "presentation_context.h"

#include "Descriptor/descriptor_heap_manager.h"
#include "Framework/Logging/logger.h"
#include "depth_buffer.h"
#include "hdr_render_target.h"
#include "swapchain_manager.h"

namespace gfx {

std::unique_ptr<PresentationContext> PresentationContext::Create(
  ID3D12Device* device, IDXGIFactory6* factory, ID3D12CommandQueue* queue, DescriptorHeapManager* heap_manager, const CreateInfo& info) {
  auto context = std::unique_ptr<PresentationContext>(new PresentationContext());
  if (!context->Initialize(device, factory, queue, heap_manager, info)) {
    return nullptr;
  }
  return context;
}

PresentationContext::~PresentationContext() {
  if (hdr_render_target_ && heap_manager_) {
    hdr_render_target_->SafeRelease(*heap_manager_);
  }
}

bool PresentationContext::Initialize(
  ID3D12Device* device, IDXGIFactory6* factory, ID3D12CommandQueue* queue, DescriptorHeapManager* heap_manager, const CreateInfo& info) {
  device_ = device;
  heap_manager_ = heap_manager;
  width_ = info.width;
  height_ = info.height;
  vsync_enabled_ = info.enable_vsync;

  swap_chain_ = std::make_unique<SwapChainManager>();
  if (!swap_chain_->Initialize(device, factory, queue, info.hwnd, info.width, info.height, *heap_manager)) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to initialize SwapChainManager");
    return false;
  }

  depth_buffer_ = std::make_unique<DepthBuffer>();
  if (!depth_buffer_->Initialize(device, info.width, info.height, *heap_manager)) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to initialize DepthBuffer");
    return false;
  }

  hdr_render_target_ = std::make_unique<HdrRenderTarget>();
  if (!hdr_render_target_->Initialize(device, info.width, info.height, *heap_manager)) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to initialize HdrRenderTarget");
    return false;
  }

  UpdateViewportAndScissor();

  Logger::LogFormat(
    LogLevel::Info, LogCategory::Graphic, Logger::Here(), "PresentationContext initialized: {}x{}", info.width, info.height);
  return true;
}

bool PresentationContext::Resize(uint32_t width, uint32_t height, ID3D12CommandQueue* queue, ID3D12Fence* fence) {
  if (width == 0 || height == 0) {
    return false;
  }

  if (width == width_ && height == height_) {
    return true;
  }

  WaitForGpu(queue, fence);

  depth_buffer_->SafeRelease();
  hdr_render_target_->SafeRelease(*heap_manager_);

  if (!swap_chain_->Resize(width, height, *heap_manager_)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "Failed to resize swap chain");
    return false;
  }

  if (!depth_buffer_->Initialize(device_, width, height, *heap_manager_)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "Failed to recreate depth buffer");
    return false;
  }

  if (!hdr_render_target_->Initialize(device_, width, height, *heap_manager_)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "Failed to recreate HDR render target");
    return false;
  }

  width_ = width;
  height_ = height;
  UpdateViewportAndScissor();

  Logger::LogFormat(LogLevel::Info, LogCategory::Graphic, Logger::Here(), "PresentationContext resized: {}x{}", width, height);
  return true;
}

void PresentationContext::Present() {
  UINT sync_interval = vsync_enabled_ ? 1 : 0;
  swap_chain_->Present(sync_interval, 0);
}

uint32_t PresentationContext::GetCurrentBackBufferIndex() const {
  return swap_chain_->GetCurrentBackBufferIndex();
}

D3D12_CPU_DESCRIPTOR_HANDLE PresentationContext::GetCurrentRTV() const {
  return swap_chain_->GetCurrentRTV();
}

D3D12_CPU_DESCRIPTOR_HANDLE PresentationContext::GetHdrRTV() const {
  return hdr_render_target_->GetRTV();
}

D3D12_CPU_DESCRIPTOR_HANDLE PresentationContext::GetDSV() const {
  return depth_buffer_->GetDSV();
}

uint32_t PresentationContext::GetHdrSrvIndex() const {
  return hdr_render_target_->GetSrvIndex();
}

void PresentationContext::BeginFrame(ID3D12GraphicsCommandList* cmd) {
  hdr_render_target_->TransitionTo(cmd, D3D12_RESOURCE_STATE_RENDER_TARGET);
  hdr_render_target_->Clear(cmd);
  depth_buffer_->Clear(cmd, 1.0f, 0);

  D3D12_CPU_DESCRIPTOR_HANDLE hdr_rtv = hdr_render_target_->GetRTV();
  D3D12_CPU_DESCRIPTOR_HANDLE dsv = depth_buffer_->GetDSV();
  cmd->OMSetRenderTargets(1, &hdr_rtv, FALSE, &dsv);

  cmd->RSSetViewports(1, &viewport_);
  cmd->RSSetScissorRects(1, &scissor_rect_);
}

void PresentationContext::EndFrame(ID3D12GraphicsCommandList* cmd) {
  TransitionSwapChainToPresent(cmd);
}

void PresentationContext::TransitionSwapChainToPresent(ID3D12GraphicsCommandList* cmd) {
  swap_chain_->TransitionToPresent(cmd);
}

void PresentationContext::TransitionHdrToShaderResource(ID3D12GraphicsCommandList* cmd) {
  hdr_render_target_->TransitionTo(cmd, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void PresentationContext::UpdateViewportAndScissor() {
  viewport_.TopLeftX = 0.0f;
  viewport_.TopLeftY = 0.0f;
  viewport_.Width = static_cast<float>(width_);
  viewport_.Height = static_cast<float>(height_);
  viewport_.MinDepth = 0.0f;
  viewport_.MaxDepth = 1.0f;

  scissor_rect_.left = 0;
  scissor_rect_.top = 0;
  scissor_rect_.right = static_cast<LONG>(width_);
  scissor_rect_.bottom = static_cast<LONG>(height_);
}

void PresentationContext::WaitForGpu(ID3D12CommandQueue* queue, ID3D12Fence* fence) {
  if (!queue || !fence) {
    return;
  }

  uint64_t current_value = fence->GetCompletedValue();
  uint64_t signal_value = current_value + 1;

  queue->Signal(fence, signal_value);

  HANDLE event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
  if (event) {
    fence->SetEventOnCompletion(signal_value, event);
    WaitForSingleObject(event, INFINITE);
    CloseHandle(event);
  }
}

}  // namespace gfx
