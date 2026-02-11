#include "presentation_context.h"

#include "Descriptor/descriptor_heap_manager.h"
#include "Framework/Logging/logger.h"
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

PresentationContext::~PresentationContext() = default;

bool PresentationContext::Initialize(
  ID3D12Device* device, IDXGIFactory6* factory, ID3D12CommandQueue* queue, DescriptorHeapManager* heap_manager, const CreateInfo& info) {
  heap_manager_ = heap_manager;
  vsync_enabled_ = info.enable_vsync;
  allow_tearing_ = info.allow_tearing;

  swap_chain_ = std::make_unique<SwapChainManager>();
  if (!swap_chain_->Initialize(device, factory, queue, info.hwnd, info.width, info.height, *heap_manager, allow_tearing_)) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to initialize SwapChainManager");
    return false;
  }

  Logger::LogFormat(
    LogLevel::Info, LogCategory::Graphic, Logger::Here(), "PresentationContext initialized: {}x{}", info.width, info.height);
  return true;
}

bool PresentationContext::Resize(uint32_t width, uint32_t height, ID3D12CommandQueue* queue, ID3D12Fence* fence) {
  if (width == 0 || height == 0) {
    return false;
  }

  WaitForGpu(queue, fence);

  if (!swap_chain_->Resize(width, height, *heap_manager_)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "Failed to resize swap chain");
    return false;
  }

  Logger::LogFormat(LogLevel::Info, LogCategory::Graphic, Logger::Here(), "PresentationContext resized: {}x{}", width, height);
  return true;
}

void PresentationContext::Present() {
  UINT sync_interval = vsync_enabled_ ? 1 : 0;
  UINT present_flags = (!vsync_enabled_ && allow_tearing_) ? DXGI_PRESENT_ALLOW_TEARING : 0;
  swap_chain_->Present(sync_interval, present_flags);
}

uint32_t PresentationContext::GetCurrentBackBufferIndex() const {
  return swap_chain_->GetCurrentBackBufferIndex();
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
