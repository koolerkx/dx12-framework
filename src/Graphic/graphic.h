#pragma once

#include <basetsd.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#include <memory>
#include <vector>

#include "Core/types.h"
#include "Debug/debug_line_renderer.h"
#include "Descriptor/descriptor_heap_manager.h"
#include "Device/device_context.h"
#include "Device/fence_manager.h"
#include "Device/frame_synchronizer.h"
#include "Frame/constant_buffers.h"
#include "Frame/dynamic_upload_buffer.h"
#include "Frame/frame_packet.h"
#include "Frame/per_frame_constant_buffer.h"
#include "Frame/render_frame_context.h"
#include "Pipeline/material_manager.h"
#include "Pipeline/shader_manager.h"
#include "Presentation/depth_buffer.h"
#include "Presentation/hdr_render_target.h"
#include "Presentation/presentation_context.h"
#include "Presentation/swapchain_manager.h"
#include "Render/material_renderer.h"
#include "Render/render_pass_manager.h"
#include "Render/tone_map_pass.h"
#include "Rendering/hdr_config.h"
#include "Resource/Font/sprite_font_manager.h"
#include "Resource/Texture/texture_manager.h"

class Graphic {
 public:
  // Simplified initialization
  struct GraphicInitProps {
    bool enable_vsync = true;
  };

  Graphic() = default;
  ~Graphic() {
    Shutdown();
  };

  bool Initialize(HWND hwnd, UINT frame_buffer_width, UINT frame_buffer_height, const GraphicInitProps& props);
  void Shutdown();

  void WaitForGpuIdle();

  // Resize graphics resources
  bool ResizeBuffers(UINT width, UINT height);

  // helper
  /**
   * @brief This function will execute a command list, the command list will record by caller in cb
   * @param cb
   */
  void ExecuteSync(std::function<void(ID3D12GraphicsCommandList*)> cb);

  RenderFrameContext BeginFrame();
  void EndFrame(const RenderFrameContext& frame);
  void RenderScene(const RenderFrameContext& frame, const FramePacket& world);

  // Debug rendering API - LOW LEVEL ONLY
  void AddDebugLine(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, const DirectX::XMFLOAT4& color = {1, 1, 1, 1});

  // Get debug line renderer for advanced use
  DebugLineRenderer* GetDebugLineRenderer() {
    return debug_line_renderer_.get();
  }

  static constexpr int FRAME_BUFFER_COUNT = 2;

  TextureManager& GetTextureManager() {
    return texture_manager_;
  }

  MaterialManager& GetMaterialManager() {
    return material_manager_;
  }

  ShaderManager& GetShaderManager() {
    return *shader_manager_;
  }

  FenceManager& GetFenceManager() {
    return fence_manager_;
  }

  Font::SpriteFontManager& GetSpriteFontManager() {
    return sprite_font_manager_;
  }

  ID3D12Device* GetDevice() const {
    return device_.Get();
  }

  void SetVSync(bool enable) {
    enable_vsync_ = enable;
  }

  gfx::DeviceContext* GetDeviceContext() const {
    return device_context_.get();
  }

  gfx::PresentationContext* GetPresentationContext() const {
    return presentation_context_.get();
  }

  gfx::FrameSynchronizer* GetFrameSynchronizer() const {
    return frame_synchronizer_.get();
  }

  DescriptorHeapManager& GetDescriptorHeapManager() {
    return descriptor_heap_manager_;
  }

 private:
  // New modular subsystems
  std::unique_ptr<gfx::DeviceContext> device_context_;
  std::unique_ptr<gfx::PresentationContext> presentation_context_;
  std::unique_ptr<gfx::FrameSynchronizer> frame_synchronizer_;

  // Core (DEPRECATED: will be replaced by subsystems)
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

  // HDR rendering
  std::unique_ptr<HdrRenderTarget> hdr_render_target_;
  std::unique_ptr<ToneMapPass> tone_map_pass_;
  HdrConfig hdr_config_;
  HdrDebug hdr_debug_;

  UINT frame_buffer_width_ = 0;
  UINT frame_buffer_height_ = 0;

  // Viewport
  D3D12_VIEWPORT viewport_ = {};
  D3D12_RECT scissor_rect_ = {};

  ConstantBuffer<FrameCB> frameCB_;
  ConstantBuffer<ObjectCB> objectCB_;

  std::array<uint64_t, FRAME_BUFFER_COUNT> frame_fence_values_ = {};

  PerFrameConstantBuffer<FrameCB> frame_cb_storage_;
  std::vector<std::unique_ptr<DynamicUploadBuffer>> object_cb_allocators_;

  // texture
  TextureManager texture_manager_;
  Font::SpriteFontManager sprite_font_manager_;
  std::unique_ptr<ShaderManager> shader_manager_;
  MaterialManager material_manager_;

  std::unique_ptr<UiRenderer> ui_renderer_;
  std::unique_ptr<OpaqueRenderer> opaque_renderer_;
  std::unique_ptr<TransparentRenderer> transparent_renderer_;
  std::unique_ptr<RenderPassManager> render_pass_manager_;

  std::unique_ptr<DebugLineRenderer> debug_line_renderer_;

  // Initialization
  bool EnableDebugLayer();
  bool CreateFactory();
  bool CreateDevice();
  bool CreateCommandQueue();

  bool CreateCommandList();
  bool CreateCommandAllocators();

  HWND hwnd_ = nullptr;
  bool enable_vsync_ = true;
  bool use_bindless_sampler_ = false;

  bool is_initialized_ = false;
  std::atomic<bool> is_shutting_down_{false};
};
