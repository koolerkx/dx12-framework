#pragma once

#include <memory>
#include <vector>

#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"
#include "Presentation/depth_buffer.h"
#include "Render/render_texture.h"
#include "render_pass.h"

class SwapChainManager;
class DescriptorHeapManager;

class RenderPassManager {
 public:
  RenderPassManager() = default;

  void SetSwapChain(SwapChainManager* swapchain);
  void SetHeapManager(DescriptorHeapManager* heap_mgr);

  RenderTexture* CreateRenderTexture(
    DXGI_FORMAT format, uint32_t width, uint32_t height, ID3D12Device* device, std::array<float, 4> clear_color = {0, 0, 0, 1});
  DepthBuffer* CreateDepthBuffer(uint32_t width, uint32_t height, ID3D12Device* device);

  void AddPass(std::unique_ptr<IRenderPass> pass);

  void BeginFrame();
  void Execute(const RenderFrameContext& frame, const FramePacket& packet);

  void Resize(ID3D12Device* device, uint32_t width, uint32_t height);
  void Shutdown();

  void FinalizeFrame(ID3D12GraphicsCommandList* cmd);

 private:
  void ApplyPassSetup(ID3D12GraphicsCommandList* cmd, const PassSetup& setup);

  std::vector<std::unique_ptr<IRenderPass>> passes_;
  std::vector<std::unique_ptr<RenderTexture>> render_textures_;
  std::unique_ptr<DepthBuffer> depth_buffer_;

  SwapChainManager* swapchain_ = nullptr;
  DescriptorHeapManager* heap_manager_ = nullptr;

  bool backbuffer_is_render_target_ = false;
  bool backbuffer_cleared_ = false;
};
