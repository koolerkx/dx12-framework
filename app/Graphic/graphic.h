#pragma once

#include <basetsd.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#include <memory>
#include <vector>

#include "Core/types.h"
#include "Texture/texture_manager.h"
#include "constant_buffers.h"
#include "depth_buffer.h"
#include "descriptor_heap_manager.h"
#include "dynamic_upload_buffer.h"
#include "fence_manager.h"
#include "frame_packet.h"
#include "mesh.h"
#include "per_frame_constant_buffer.h"
#include "render_frame_context.h"
#include "render_pass_manager.h"
#include "swapchain_manager.h"
#include "ui_pass.h"
#include "ui_renderer.h"

struct GraphicInitProps {
  bool enable_vsync = true;
};

class Graphic {
 public:
  Graphic() = default;
  ~Graphic() {
    Shutdown();
  };

  bool Initialize(HWND hwnd, UINT frame_buffer_width, UINT frame_buffer_height, GraphicInitProps props = {});
  void Shutdown();

  // helper
  /**
   * @brief This function will execute a command list, the command list will record by caller in cb
   * @param cb
   */
  void ExecuteSync(std::function<void(ID3D12GraphicsCommandList*)> cb);

  RenderFrameContext BeginFrame();
  void EndFrame(const RenderFrameContext& frame);
  void RenderScene(const RenderFrameContext& frame, const FramePacket& world);

  static constexpr int FRAME_BUFFER_COUNT = 2;

  TextureManager& GetTextureManager() {
    return texture_manager_;
  }

  FenceManager& GetFenceManager() {
    return fence_manager_;
  }

  void SetVSync(bool enable) {
    enable_vsync_ = enable;
  }

 private:
  // Core
  ComPtr<ID3D12Device5> device_ = nullptr;  /// @note D3D Device, RTX graphic card required
  ComPtr<IDXGIFactory6> dxgi_factory_ = nullptr;

  ComPtr<ID3D12CommandAllocator> utility_command_allocator_ = nullptr;                 // for ExecuteSync
  std::array<ComPtr<ID3D12CommandAllocator>, FRAME_BUFFER_COUNT> command_allocators_;  // for frame in flight
  ComPtr<ID3D12GraphicsCommandList> command_list_ = nullptr;
  ComPtr<ID3D12CommandQueue> command_queue_ = nullptr;

  // descriptor management
  DescriptorHeapManager descriptor_heap_manager_;
  SwapChainManager swap_chain_manager_;
  DepthBuffer depth_buffer_;
  FenceManager fence_manager_;

  UINT frame_buffer_width_ = 0;
  UINT frame_buffer_height_ = 0;

  // Pipeline State
  ComPtr<ID3D12RootSignature> root_signature_ = nullptr;
  ComPtr<ID3D12PipelineState> pipeline_state_ = nullptr;

  // Viewport
  D3D12_VIEWPORT viewport_ = {};
  D3D12_RECT scissor_rect_ = {};

  Mesh quadMesh_;

  ConstantBuffer<FrameCB> frameCB_;
  ConstantBuffer<ObjectCB> objectCB_;

  std::array<uint64_t, FRAME_BUFFER_COUNT> frame_fence_values_ = {};

  PerFrameConstantBuffer<FrameCB> frame_cb_storage_;
  std::vector<std::unique_ptr<DynamicUploadBuffer>> object_cb_allocators_;

  // texture
  TextureManager texture_manager_;

  std::unique_ptr<UiRenderer> ui_renderer_;
  std::unique_ptr<UiPass> ui_pass_;
  std::unique_ptr<RenderPassManager> render_pass_manager_;

  // Initialization
  bool EnableDebugLayer();
  bool CreateFactory();
  bool CreateDevice();
  bool CreateCommandQueue();

  bool CreateCommandList();
  bool CreateCommandAllocators();

  bool enable_vsync_ = true;

  bool is_initialized_ = false;
  std::atomic<bool> is_shutting_down_{false};
};
