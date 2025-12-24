#pragma once

#include <basetsd.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#include <memory>
#include <vector>

#include "Texture/texture_manager.h"
#include "depth_buffer.h"
#include "descriptor_heap_manager.h"
#include "fence_manager.h"
#include "mesh.h"
#include "swapchain_manager.h"
#include "types.h"


class Graphic {
 public:
  Graphic() = default;
  ~Graphic() = default;

  bool Initalize(HWND hwnd, UINT frame_buffer_width, UINT frame_buffer_height);
  void BeginRender();
  void EndRender();
  void Shutdown();

  // helper
  void ExecuteSync(std::function<void(ID3D12GraphicsCommandList*)> cb);

  static constexpr int FRAME_BUFFER_COUNT = 2;

 private:
  // Core
  ComPtr<ID3D12Device5> device_ = nullptr;  /// @note D3D Device, RTX graphic card required
  ComPtr<IDXGIFactory6> dxgi_factory_ = nullptr;

  ComPtr<ID3D12CommandAllocator> command_allocator_ = nullptr;
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

  // texture
  TextureManager texture_manager_;
  std::vector<std::shared_ptr<Texture>> myTexture;

  // Initialization
  bool EnableDebugLayer();
  bool CreateFactory();
  bool CreateDevice();
  bool CreateCommandQueue();

  bool CreateCommandList();
  bool CreateCommandAllocator();
};
