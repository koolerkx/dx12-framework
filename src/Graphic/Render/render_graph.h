#pragma once

#include <memory>
#include <vector>

#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"
#include "Render/render_graph_handle.h"
#include "Render/render_pass.h"

class RenderTexture;
class DepthBuffer;
class SwapChainManager;
class DescriptorHeapManager;

enum class RenderGraphResourceType : uint8_t { RenderTexture, DepthBuffer, Backbuffer };

class RenderGraph {
 public:
  RenderGraph();
  ~RenderGraph();

  void SetSwapChain(SwapChainManager* swapchain);
  void SetHeapManager(DescriptorHeapManager* heap_mgr);

  RenderGraphHandle CreateRenderTexture(
    const char* name, DXGI_FORMAT format, uint32_t width, uint32_t height, ID3D12Device* device, Color clear_color = colors::Black);
  RenderGraphHandle CreateDepthBuffer(const char* name, uint32_t width, uint32_t height, ID3D12Device* device);
  RenderGraphHandle ImportBackbuffer(const char* name);

  uint32_t GetSrvIndex(RenderGraphHandle handle) const;

  void AddPass(std::unique_ptr<IRenderPass> pass);

  void BeginFrame();
  void Execute(const RenderFrameContext& frame, const FramePacket& packet);
  void FinalizeFrame(ID3D12GraphicsCommandList* cmd);
  void Resize(ID3D12Device* device, uint32_t width, uint32_t height);
  void Shutdown();

 private:
  struct ResourceEntry {
    const char* name = nullptr;
    RenderGraphResourceType type;
    RenderTexture* render_texture = nullptr;
    DepthBuffer* depth_buffer = nullptr;
  };

  void ApplyPassSetup(ID3D12GraphicsCommandList* cmd, const PassSetup& setup);
  const ResourceEntry& GetEntry(RenderGraphHandle handle) const;

  std::vector<ResourceEntry> resources_;
  std::vector<std::unique_ptr<RenderTexture>> owned_render_textures_;
  std::vector<std::unique_ptr<DepthBuffer>> owned_depth_buffers_;
  std::vector<std::unique_ptr<IRenderPass>> passes_;

  SwapChainManager* swapchain_ = nullptr;
  DescriptorHeapManager* heap_manager_ = nullptr;
  bool backbuffer_is_render_target_ = false;
  bool backbuffer_cleared_ = false;
};
