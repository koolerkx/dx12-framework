/**
 * @file presentation_context.h
 * @brief Manages the swap chain, depth and HDR render targets, viewport/scissor,
 * and the frame presentation lifecycle, including GPU synchronization and resize handling.
 */

#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

#include <cstdint>
#include <memory>

class DescriptorHeapManager;
class SwapChainManager;
class DepthBuffer;
class HdrRenderTarget;

namespace gfx {

class PresentationContext {
 public:
  struct CreateInfo {
    HWND hwnd = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t buffer_count = 2;
    bool enable_vsync = true;
  };

  [[nodiscard]] static std::unique_ptr<PresentationContext> Create(
    ID3D12Device* device, IDXGIFactory6* factory, ID3D12CommandQueue* queue, DescriptorHeapManager* heap_manager, const CreateInfo& info);

  PresentationContext(const PresentationContext&) = delete;
  PresentationContext& operator=(const PresentationContext&) = delete;
  ~PresentationContext();

  bool Resize(uint32_t width, uint32_t height, ID3D12CommandQueue* queue, ID3D12Fence* fence);
  void Present();

  uint32_t GetCurrentBackBufferIndex() const;
  uint32_t GetWidth() const {
    return width_;
  }
  uint32_t GetHeight() const {
    return height_;
  }

  D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRTV() const;
  D3D12_CPU_DESCRIPTOR_HANDLE GetHdrRTV() const;
  D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const;
  uint32_t GetHdrSrvIndex() const;

  const D3D12_VIEWPORT& GetViewport() const {
    return viewport_;
  }
  const D3D12_RECT& GetScissorRect() const {
    return scissor_rect_;
  }

  void SetVSync(bool enable) {
    vsync_enabled_ = enable;
  }
  bool IsVSyncEnabled() const {
    return vsync_enabled_;
  }

  void BeginFrame(ID3D12GraphicsCommandList* cmd);
  void EndFrame(ID3D12GraphicsCommandList* cmd);

  void TransitionSwapChainToPresent(ID3D12GraphicsCommandList* cmd);
  void TransitionHdrToShaderResource(ID3D12GraphicsCommandList* cmd);

  SwapChainManager& GetSwapChainManager() {
    return *swap_chain_;
  }
  HdrRenderTarget& GetHdrRenderTarget() {
    return *hdr_render_target_;
  }
  DepthBuffer& GetDepthBuffer() {
    return *depth_buffer_;
  }

 private:
  PresentationContext() = default;
  bool Initialize(
    ID3D12Device* device, IDXGIFactory6* factory, ID3D12CommandQueue* queue, DescriptorHeapManager* heap_manager, const CreateInfo& info);

  void UpdateViewportAndScissor();
  void WaitForGpu(ID3D12CommandQueue* queue, ID3D12Fence* fence);

  std::unique_ptr<SwapChainManager> swap_chain_;
  std::unique_ptr<DepthBuffer> depth_buffer_;
  std::unique_ptr<HdrRenderTarget> hdr_render_target_;
  DescriptorHeapManager* heap_manager_ = nullptr;
  ID3D12Device* device_ = nullptr;

  uint32_t width_ = 0;
  uint32_t height_ = 0;
  bool vsync_enabled_ = true;

  D3D12_VIEWPORT viewport_{};
  D3D12_RECT scissor_rect_{};
};

}  // namespace gfx
