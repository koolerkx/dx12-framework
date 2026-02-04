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

  // Create PresentationContext
  gfx::PresentationContext::CreateInfo presentation_info{};
  presentation_info.hwnd = hwnd;
  presentation_info.width = frame_buffer_width;
  presentation_info.height = frame_buffer_height;
  presentation_info.buffer_count = FRAME_BUFFER_COUNT;
  presentation_info.enable_vsync = props.enable_vsync;
  presentation_context_ = gfx::PresentationContext::Create(
    device_.Get(), dxgi_factory_.Get(), command_queue_.Get(), &descriptor_heap_manager_, presentation_info);
  if (!presentation_context_) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to create PresentationContext");
    return false;
  }

  // Create tone mapping pass and set up dependencies
  tone_map_pass_ = std::make_unique<ToneMapPass>(device_.Get(), &material_manager_, shader_manager_.get());
  tone_map_pass_->SetDependencies(presentation_context_.get(), &hdr_config_, &hdr_debug_);
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

  if (!presentation_context_->Resize(width, height, command_queue_.Get(), frame_synchronizer_->GetFence())) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "PresentationContext resize failed");
    return false;
  }

  frame_buffer_width_ = width;
  frame_buffer_height_ = height;

  Logger::LogFormat(LogLevel::Info, LogCategory::Graphic, Logger::Here(), "Resized to {}x{}", width, height);
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

  // Explicitly release subsystems that depend on descriptor_heap_manager_
  // Must happen before descriptor_heap_manager_ is destroyed (reverse declaration order)
  presentation_context_.reset();

  // Resources released by ComPtr destructors
  is_initialized_ = false;
  is_shutting_down_.store(false, std::memory_order_release);
}

RenderFrameContext Graphic::BeginFrame() {
  uint32_t frame_index = presentation_context_->GetCurrentBackBufferIndex();

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

  // Delegate HDR RT, depth buffer, viewport setup to PresentationContext
  presentation_context_->BeginFrame(cmd);

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
  presentation_context_->EndFrame(frame.command_list);

  command_context_->Execute();

  uint64_t fence_value = frame_synchronizer_->SignalFrame(command_queue_.Get(), frame.frame_index);
  frame_cb_storage_.MarkFrameSubmitted(frame.frame_index, fence_value);

  texture_manager_.CleanUploadBuffers();

  material_manager_.OnFrameEnd();

  presentation_context_->Present();
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

void Graphic::ExecuteSync(std::function<void(ID3D12GraphicsCommandList*)> cb) {
  command_context_->ExecuteSync(frame_synchronizer_->GetFence(), cb);
}
