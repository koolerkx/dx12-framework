#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

#include <cstdint>
#include <memory>

class DescriptorHeapManager;
class SwapChainManager;

namespace gfx {

class PresentationContext {
 public:
  struct CreateInfo {
    HWND hwnd = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t buffer_count = 2;
    bool enable_vsync = true;
    bool allow_tearing = false;
  };

  [[nodiscard]] static std::unique_ptr<PresentationContext> Create(
    ID3D12Device* device, IDXGIFactory6* factory, ID3D12CommandQueue* queue, DescriptorHeapManager* heap_manager, const CreateInfo& info);

  PresentationContext(const PresentationContext&) = delete;
  PresentationContext& operator=(const PresentationContext&) = delete;
  ~PresentationContext();

  bool Resize(uint32_t width, uint32_t height, ID3D12CommandQueue* queue, ID3D12Fence* fence);
  void Present();

  uint32_t GetCurrentBackBufferIndex() const;

  void SetVSync(bool enable) { vsync_enabled_ = enable; }
  bool IsVSyncEnabled() const { return vsync_enabled_; }

  SwapChainManager& GetSwapChainManager() { return *swap_chain_; }

 private:
  PresentationContext() = default;
  bool Initialize(
    ID3D12Device* device, IDXGIFactory6* factory, ID3D12CommandQueue* queue, DescriptorHeapManager* heap_manager, const CreateInfo& info);

  void WaitForGpu(ID3D12CommandQueue* queue, ID3D12Fence* fence);

  std::unique_ptr<SwapChainManager> swap_chain_;
  DescriptorHeapManager* heap_manager_ = nullptr;

  bool vsync_enabled_ = true;
  bool allow_tearing_ = false;
};

}  // namespace gfx
