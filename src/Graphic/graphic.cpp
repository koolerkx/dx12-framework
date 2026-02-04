#include "graphic.h"

#include <DirectXMath.h>
#include <d3d12.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <dxgiformat.h>

#include <cstdint>
#include <vector>

#include "Core/types.h"
#include "Frame/frame_packet.h"
#include "Framework/Logging/logger.h"
#include "Render/debug_pass.h"
#include "Render/opaque_pass.h"
#include "Render/transparent_pass.h"
#include "Render/ui_pass.h"

using namespace DirectX;

constexpr std::array<float, 4> CLEAR_COLOR = {1.0f, 1.0f, 0.0f, 1.0f};

struct Vertex {
  XMFLOAT3 pos;
  XMFLOAT2 uv;
};

struct TexRGBA {
  unsigned char R, G, B, A;
};

bool Graphic::Initialize(HWND hwnd, UINT frame_buffer_width, UINT frame_buffer_height, const Graphic::GraphicInitProps& props) {
  // Cleanup
  Shutdown();
  is_shutting_down_ = false;

  hwnd_ = hwnd;
  frame_buffer_width_ = frame_buffer_width;
  frame_buffer_height_ = frame_buffer_height;
  enable_vsync_ = props.enable_vsync;

  std::wstring init_error_caption = L"Graphic Initialization Error";

  // Create DeviceContext (new modular subsystem)
  gfx::DeviceContext::CreateInfo device_info{};
#if defined(DEBUG) || defined(_DEBUG)
  device_info.enable_debug_layer = true;
#endif
  device_context_ = gfx::DeviceContext::Create(device_info);
  if (!device_context_) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to create DeviceContext");
    MessageBoxW(nullptr, L"Graphic: Failed to create DeviceContext", init_error_caption.c_str(), MB_OK | MB_ICONERROR);
    return false;
  }

  // Copy references for backward compatibility
  device_ = device_context_->GetDevice();
  dxgi_factory_ = device_context_->GetFactory();
  use_bindless_sampler_ = device_context_->SupportsBindlessSamplers();

  DescriptorHeapConfig heapConfig;
  if (!descriptor_heap_manager_.Initialize(device_.Get(), FRAME_BUFFER_COUNT, heapConfig)) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to initialize descriptor heap manager");
    MessageBoxW(nullptr, L"Graphic: Failed to initialize descriptor heap manager", init_error_caption.c_str(), MB_OK | MB_ICONERROR);
    return false;
  }

  // Create CommandContext: Command List, Command Allocator, Command Queue
  command_context_ = gfx::CommandContext::Create(device_.Get());
  if (!command_context_) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to create CommandContext");
    MessageBoxW(nullptr, L"Graphic: Failed to create CommandContext", init_error_caption.c_str(), MB_OK | MB_ICONERROR);
    return false;
  }

  // Copy reference for backward compatibility (SwapChainManager needs command_queue_)
  command_queue_ = command_context_->GetQueue();

  if (!swap_chain_manager_.Initialize(device_.Get(),
        dxgi_factory_.Get(),
        command_queue_.Get(),
        hwnd,
        frame_buffer_width,
        frame_buffer_height,
        descriptor_heap_manager_)) {
    return false;
  }
  if (!depth_buffer_.Initialize(device_.Get(), frame_buffer_width, frame_buffer_height, descriptor_heap_manager_)) {
    return false;
  }

  // Create FrameSynchronizer
  frame_synchronizer_ = gfx::FrameSynchronizer::Create(device_.Get());
  if (!frame_synchronizer_) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to create FrameSynchronizer");
    return false;
  }

  if (!texture_manager_.Initialize(this, device_.Get(), &descriptor_heap_manager_)) {
    return false;
  }

  shader_manager_ = std::make_unique<ShaderManager>();
  if (!shader_manager_->Initialize(device_.Get())) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "Failed to initialize ShaderManager");
    return false;
  }

  if (!material_manager_.Initialize(device_.Get(), shader_manager_.get())) {
    return false;
  }

  if (!sprite_font_manager_.Initialize(&texture_manager_, device_.Get())) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "Failed to initialize SpriteFontManager");
    return false;
  }
  Font::LoadDefaultFonts(sprite_font_manager_);

  if (!frame_cb_storage_.Initialize(device_.Get(), FRAME_BUFFER_COUNT)) return false;

  size_t object_buffer_page_size = 1024 * 1024;  // 1mb per page

  object_cb_allocators_.resize(FRAME_BUFFER_COUNT);
  for (int i = 0; i < FRAME_BUFFER_COUNT; ++i) {
    object_cb_allocators_[i] = std::make_unique<DynamicUploadBuffer>();
    std::wstring bufferName = L"ObjectCB_Frame" + std::to_wstring(i);
    if (!object_cb_allocators_[i]->Initialize(device_.Get(), object_buffer_page_size, bufferName)) {
      return false;
    }
  }

  viewport_.Width = static_cast<FLOAT>(frame_buffer_width_);    // 出力先の幅(ピクセル数)
  viewport_.Height = static_cast<FLOAT>(frame_buffer_height_);  // 出力先の高さ(ピクセル数)
  viewport_.TopLeftX = 0;                                       // 出力先の左上座標X
  viewport_.TopLeftY = 0;                                       // 出力先の左上座標Y
  viewport_.MaxDepth = 1.0f;                                    // 深度最大値
  viewport_.MinDepth = 0.0f;                                    // 深度最小値

  scissor_rect_.top = 0;                                            // 切り抜き上座標
  scissor_rect_.left = 0;                                           // 切り抜き左座標
  scissor_rect_.right = scissor_rect_.left + frame_buffer_width_;   // 切り抜き右座標
  scissor_rect_.bottom = scissor_rect_.top + frame_buffer_height_;  // 切り抜き下座標

  ui_renderer_ = std::make_unique<UiRenderer>();
  opaque_renderer_ = std::make_unique<OpaqueRenderer>();
  transparent_renderer_ = std::make_unique<TransparentRenderer>();

  debug_line_renderer_ = std::make_unique<DebugLineRenderer>();
  if (!debug_line_renderer_->Initialize(device_.Get())) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "Failed to initialize debug line renderer");
    return false;
  }

  render_pass_manager_ = std::make_unique<RenderPassManager>();

  render_pass_manager_->AddPass(std::make_unique<OpaquePass>(opaque_renderer_.get()));
  render_pass_manager_->AddPass(std::make_unique<TransparentPass>(transparent_renderer_.get()));
  render_pass_manager_->AddPass(std::make_unique<DebugPass>(debug_line_renderer_.get(), &material_manager_));

  // Create HDR render target
  hdr_render_target_ = std::make_unique<HdrRenderTarget>();
  if (!hdr_render_target_->Initialize(device_.Get(), frame_buffer_width, frame_buffer_height, descriptor_heap_manager_)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "Failed to initialize HDR render target");
    return false;
  }

  // Create PresentationContext (new modular subsystem - for future use)
  gfx::PresentationContext::CreateInfo presentation_info{};
  presentation_info.hwnd = hwnd;
  presentation_info.width = frame_buffer_width;
  presentation_info.height = frame_buffer_height;
  presentation_info.buffer_count = FRAME_BUFFER_COUNT;
  presentation_info.enable_vsync = props.enable_vsync;
  presentation_context_ = gfx::PresentationContext::Create(
    device_.Get(), dxgi_factory_.Get(), command_queue_.Get(), &descriptor_heap_manager_, presentation_info);
  if (!presentation_context_) {
    Logger::LogFormat(
      LogLevel::Warn, LogCategory::Graphic, Logger::Here(), "PresentationContext creation failed - using legacy presentation path");
  }

  // Create tone mapping pass and set up dependencies
  tone_map_pass_ = std::make_unique<ToneMapPass>(device_.Get(), &material_manager_, shader_manager_.get());
  tone_map_pass_->SetDependencies(hdr_render_target_.get(), &swap_chain_manager_, &hdr_config_, &hdr_debug_);
  render_pass_manager_->AddPass(std::move(tone_map_pass_));

  render_pass_manager_->AddPass(std::make_unique<UiPass>(ui_renderer_.get()));

  is_initialized_ = true;
  return true;
}

bool Graphic::ResizeBuffers(UINT width, UINT height) {
  if (!is_initialized_ || width == 0 || height == 0) {
    return false;
  }

  if (width == frame_buffer_width_ && height == frame_buffer_height_) {
    return true;  // Already at target size
  }

  frame_synchronizer_->WaitForGpuIdle(command_queue_.Get());
  depth_buffer_.SafeRelease();

  // Release HDR RT (CRITICAL: pass descriptor_manager to free SRV)
  hdr_render_target_->SafeRelease(descriptor_heap_manager_);

  // Resize swap chain
  if (!swap_chain_manager_.Resize(width, height, descriptor_heap_manager_)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "Failed to resize swap chain");
    return false;
  }

  // Recreate depth buffer
  if (!depth_buffer_.Initialize(device_.Get(), width, height, descriptor_heap_manager_)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "Failed to recreate depth buffer");
    return false;
  }

  // Recreate HDR RT
  if (!hdr_render_target_->Initialize(device_.Get(), width, height, descriptor_heap_manager_)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "Failed to recreate HDR render target");
    return false;
  }

  frame_buffer_width_ = width;
  frame_buffer_height_ = height;
  viewport_.Width = static_cast<FLOAT>(width);
  viewport_.Height = static_cast<FLOAT>(height);
  scissor_rect_.right = width;
  scissor_rect_.bottom = height;

  return true;
}

void Graphic::WaitForGpuIdle() {
  if (command_queue_ && frame_synchronizer_ && frame_synchronizer_->IsValid()) {
    frame_synchronizer_->WaitForGpuIdle(command_queue_.Get());
  }
}

void Graphic::Shutdown() {
  bool expected = false;
  if (!is_shutting_down_.compare_exchange_strong(expected, true)) {
    return;
  }

  bool gpu_synced = false;

  try {
    if (command_queue_ && frame_synchronizer_ && frame_synchronizer_->IsValid()) {
      frame_synchronizer_->WaitForGpuIdle(command_queue_.Get());
      gpu_synced = true;

      uint64_t completed = frame_synchronizer_->GetCompletedValue();
      texture_manager_.ProcessDeferredFrees(completed);
      texture_manager_.CleanUploadBuffers();
    }
  } catch (const std::system_error& e) {
    Logger::LogFormat(
      LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[Graphic::Shutdown] System error: {} (code: {})", e.what(), e.code().value());
  } catch (const std::exception& e) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[Graphic::Shutdown] Exception: {}", e.what());
  } catch (...) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[Graphic::Shutdown] Unknown exception");
  }

  if (!gpu_synced) {
    Logger::LogFormat(
      LogLevel::Warn, LogCategory::Graphic, Logger::Here(), "[Graphic::Shutdown] WARNING: GPU sync failed, potential resource leak");
  }

  // Resources released by ComPtr destructors
  is_initialized_ = false;
  is_shutting_down_.store(false, std::memory_order_release);
}

RenderFrameContext Graphic::BeginFrame() {
  uint32_t frame_index = swap_chain_manager_.GetCurrentBackBufferIndex();

  // Clear debug lines at start of frame
  if (debug_line_renderer_) {
    debug_line_renderer_->Clear();
  }

  // Wait for previous frame using this buffer to complete
  frame_synchronizer_->WaitForFrame(frame_index);
  uint64_t completed_fence = frame_synchronizer_->GetCompletedValue();
  texture_manager_.ProcessDeferredFrees(completed_fence);

  object_cb_allocators_[frame_index]->Reset();  // manually reset for dynamic allocation

  command_context_->BeginFrame(frame_index);
  auto* cmd = command_context_->GetCommandList();

  descriptor_heap_manager_.BeginFrame(frame_index);
  descriptor_heap_manager_.SetDescriptorHeaps(cmd);  // Bind Global Heap

  // HDR workflow: Transition HDR RT to RENDER_TARGET and clear to black
  hdr_render_target_->TransitionTo(cmd, D3D12_RESOURCE_STATE_RENDER_TARGET);
  hdr_render_target_->Clear(cmd);

  // Clear depth buffer (unchanged)
  depth_buffer_.Clear(cmd, 1.0, 0);

  // Set HDR RT as render target (swapchain remains in PRESENT state)
  D3D12_CPU_DESCRIPTOR_HANDLE hdr_rtv = hdr_render_target_->GetRTV();
  D3D12_CPU_DESCRIPTOR_HANDLE dsv = depth_buffer_.GetDSV();
  cmd->OMSetRenderTargets(1, &hdr_rtv, FALSE, &dsv);

  cmd->RSSetViewports(1, &viewport_);
  cmd->RSSetScissorRects(1, &scissor_rect_);

  return RenderFrameContext{.frame_index = frame_index,
    .command_list = cmd,

    // Pass the pointer to the specific ConstantBuffer instance
    .frame_cb = &frame_cb_storage_.GetBuffer(frame_index),
    .object_cb_allocator = object_cb_allocators_[frame_index].get(),

    .dynamic_allocator = &descriptor_heap_manager_.GetSrvDynamicAllocator(frame_index),
    .global_heap_manager = &descriptor_heap_manager_,

    .screen_width = frame_buffer_width_,
    .screen_height = frame_buffer_height_};
}

void Graphic::EndFrame(const RenderFrameContext& frame) {
  swap_chain_manager_.TransitionToPresent(frame.command_list);

  command_context_->Execute();

  uint64_t fence_value = frame_synchronizer_->SignalFrame(command_queue_.Get(), frame.frame_index);
  frame_cb_storage_.MarkFrameSubmitted(frame.frame_index, fence_value);

  texture_manager_.CleanUploadBuffers();

  material_manager_.OnFrameEnd();

  UINT sync_interval = enable_vsync_ ? 1 : 0;
  swap_chain_manager_.Present(sync_interval, 0);
}

void Graphic::RenderScene(const RenderFrameContext& frame, const FramePacket& world) {
  // All passes managed uniformly through RenderPassManager
  render_pass_manager_->Execute(frame, world);
}

void Graphic::AddDebugLine(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, const DirectX::XMFLOAT4& color) {
  if (debug_line_renderer_) {
    debug_line_renderer_->AddLine(start, end, color);
  }
}

bool Graphic::EnableDebugLayer() {
  ID3D12Debug* debugLayer = nullptr;

  if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer)))) {
    return false;
  }

  debugLayer->EnableDebugLayer();
  debugLayer->Release();

  return true;
}

bool Graphic::CreateFactory() {
#if defined(DEBUG) || defined(_DEBUG)
  EnableDebugLayer();
  UINT dxgi_factory_flag = DXGI_CREATE_FACTORY_DEBUG;
#else
  UINT dxgi_factory_flag = 0;
#endif

  auto hr = CreateDXGIFactory2(dxgi_factory_flag, IID_PPV_ARGS(&dxgi_factory_));
  if (FAILED(hr)) {
    return false;
  }
  return true;
}

bool Graphic::CreateDevice() {
  std::vector<ComPtr<IDXGIAdapter>> adapters;
  ComPtr<IDXGIAdapter> tmpAdapter = nullptr;

  for (int i = 0; dxgi_factory_->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
    adapters.push_back(tmpAdapter);
  }

  for (auto adapter : adapters) {
    DXGI_ADAPTER_DESC adapter_desc = {};
    adapter->GetDesc(&adapter_desc);
    if (std::wstring desc_str = adapter_desc.Description; desc_str.find(L"NVIDIA") != std::string::npos) {
      tmpAdapter = adapter;
      break;
    }
  }

  D3D_FEATURE_LEVEL levels[] = {
    D3D_FEATURE_LEVEL_12_2,
    D3D_FEATURE_LEVEL_12_1,
    D3D_FEATURE_LEVEL_12_0,
    D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0,
  };

  for (auto level : levels) {
    auto hr = D3D12CreateDevice(tmpAdapter.Get(), level, IID_PPV_ARGS(&device_));
    if (SUCCEEDED(hr) && device_ != nullptr) {
      // Check bindless sampler support (Tier 3)
      D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
      if (SUCCEEDED(device_->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options)))) {
        use_bindless_sampler_ = (options.ResourceBindingTier >= D3D12_RESOURCE_BINDING_TIER_3);
      } else {
        use_bindless_sampler_ = false;
        Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "Not support bindless sampler support");
        return false;
      }

      return true;
    }
  }

  Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "Failed to create device");
  return false;
}

void Graphic::ExecuteSync(std::function<void(ID3D12GraphicsCommandList*)> cb) {
  command_context_->ExecuteSync(frame_synchronizer_->GetFence(), cb);
}
