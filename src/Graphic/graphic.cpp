#include "graphic.h"

#include <d3d12.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <dxgiformat.h>

#include <cstdint>
#include <vector>

#include "Core/types.h"
#include "Frame/frame_packet.h"
#include "Framework/Logging/logger.h"
#include "Framework/Math/Math.h"
#include "Presentation/swapchain_manager.h"
#include "Render/blit_pass.h"
#include "Render/debug_pass.h"
#include "Render/depth_view_pass.h"
#include "Render/material_pass.h"
#include "Render/skybox_pass.h"
#include "Render/tone_map_pass.h"

using Math::Vector3;
using Math::Vector4;

bool Graphic::Initialize(HWND hwnd, UINT frame_buffer_width, UINT frame_buffer_height, const Graphic::GraphicInitProps& props) {
  Shutdown();
  is_shutting_down_ = false;

  hwnd_ = hwnd;
  frame_buffer_width_ = frame_buffer_width;
  frame_buffer_height_ = frame_buffer_height;
  enable_vsync_ = props.enable_vsync;

  std::wstring init_error_caption = L"Graphic Initialization Error";

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

  device_ = device_context_->GetDevice();
  dxgi_factory_ = device_context_->GetFactory();
  use_bindless_sampler_ = device_context_->SupportsBindlessSamplers();

  DescriptorHeapConfig heapConfig;
  if (!descriptor_heap_manager_.Initialize(device_.Get(), FRAME_BUFFER_COUNT, heapConfig)) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to initialize descriptor heap manager");
    MessageBoxW(nullptr, L"Graphic: Failed to initialize descriptor heap manager", init_error_caption.c_str(), MB_OK | MB_ICONERROR);
    return false;
  }

  command_context_ = gfx::CommandContext::Create(device_.Get());
  if (!command_context_) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to create CommandContext");
    MessageBoxW(nullptr, L"Graphic: Failed to create CommandContext", init_error_caption.c_str(), MB_OK | MB_ICONERROR);
    return false;
  }

  command_queue_ = command_context_->GetQueue();

  frame_synchronizer_ = gfx::FrameSynchronizer::Create(device_.Get());
  if (!frame_synchronizer_) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to create FrameSynchronizer");
    return false;
  }

  gfx::RenderServices::CreateInfo services_info{};
  services_info.device = device_.Get();
  services_info.heap_manager = &descriptor_heap_manager_;
  services_info.execute_sync = [this](auto cb) { ExecuteSync(std::move(cb)); };
  services_info.get_current_fence_value = [this]() { return frame_synchronizer_->GetFenceManager().GetCurrentFenceValue(); };
  services_info.frame_buffer_count = FRAME_BUFFER_COUNT;

  render_services_ = gfx::RenderServices::Create(services_info);
  if (!render_services_) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to create RenderServices");
    return false;
  }

  if (!frame_cb_storage_.Initialize(device_.Get(), FRAME_BUFFER_COUNT)) return false;

  size_t object_buffer_page_size = 1024 * 1024;

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

  // Create PresentationContext (swapchain only)
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

  render_graph_ = std::make_unique<RenderGraph>();
  render_graph_->SetSwapChain(&presentation_context_->GetSwapChainManager());
  render_graph_->SetHeapManager(&descriptor_heap_manager_);

  auto backbuffer = render_graph_->ImportBackbuffer("backbuffer");
  auto scene_rt = render_graph_->CreateRenderTexture(
    "scene_rt", DXGI_FORMAT_R16G16B16A16_FLOAT, frame_buffer_width, frame_buffer_height, device_.Get(), colors::DarkSlateGray);
  auto scene_depth = render_graph_->CreateDepthBuffer("scene_depth", frame_buffer_width, frame_buffer_height, device_.Get());
  auto tonemap_rt =
    render_graph_->CreateRenderTexture("tonemap_rt", DXGI_FORMAT_R8G8B8A8_UNORM, frame_buffer_width, frame_buffer_height, device_.Get());
  auto depth_preview_rt = render_graph_->CreateRenderTexture(
    "depth_preview_rt", DXGI_FORMAT_R8G8B8A8_UNORM, frame_buffer_width, frame_buffer_height, device_.Get());

  preview_handles_ = {scene_rt, depth_preview_rt, tonemap_rt};

  PassSetup scene_setup;
  scene_setup.resource_writes = {scene_rt};
  scene_setup.depth = scene_depth;

  PassSetup backbuffer_setup;
  backbuffer_setup.resource_writes = {backbuffer};

  PassSetup tonemap_setup;
  tonemap_setup.resource_writes = {tonemap_rt};
  tonemap_setup.resource_reads = {scene_rt};

  render_graph_->AddPass(std::make_unique<SkyboxPass>(device_.Get(), &render_services_->GetShaderManager(), scene_setup));
  render_graph_->AddPass(std::make_unique<MaterialPass>("Opaque Pass", opaque_renderer_.get(), RenderLayer::Opaque, scene_setup));
  render_graph_->AddPass(
    std::make_unique<MaterialPass>("Transparent Pass", transparent_renderer_.get(), RenderLayer::Transparent, scene_setup));
  render_graph_->AddPass(std::make_unique<DebugPass>(debug_line_renderer_.get(), &render_services_->GetMaterialManager(), scene_setup));
  render_graph_->AddPass(std::make_unique<ToneMapPass>(device_.Get(),
    &render_services_->GetMaterialManager(),
    &render_services_->GetShaderManager(),
    tonemap_setup,
    scene_rt,
    &hdr_config_,
    &hdr_debug_));

  PassSetup depth_preview_setup;
  depth_preview_setup.resource_writes = {depth_preview_rt};
  depth_preview_setup.resource_reads = {scene_depth};

  render_graph_->AddPass(std::make_unique<DepthViewPass>(
    device_.Get(), &render_services_->GetShaderManager(), depth_preview_setup, scene_depth, &depth_preview_config_));

  PassSetup blit_setup;
  blit_setup.resource_writes = {backbuffer};
  blit_setup.resource_reads = {tonemap_rt};

  render_graph_->AddPass(std::make_unique<BlitPass>(device_.Get(), &render_services_->GetShaderManager(), blit_setup, tonemap_rt));

  PassSetup depth_view_setup;
  depth_view_setup.resource_writes = {backbuffer};
  depth_view_setup.resource_reads = {scene_depth};

  render_graph_->AddPass(std::make_unique<DepthViewPass>(
    device_.Get(), &render_services_->GetShaderManager(), depth_view_setup, scene_depth, &depth_view_config_));

  auto ui_camera_from_packet = [](const RenderFrameContext&, const FramePacket& packet) { return packet.ui_camera; };
  render_graph_->AddPass(
    std::make_unique<MaterialPass>("UI Pass", ui_renderer_.get(), RenderLayer::UI, backbuffer_setup, ui_camera_from_packet));

  is_initialized_ = true;
  return true;
}

bool Graphic::ResizeBuffers(UINT width, UINT height) {
  if (!is_initialized_ || width == 0 || height == 0) {
    return false;
  }

  if (width == frame_buffer_width_ && height == frame_buffer_height_) {
    return true;
  }

  frame_synchronizer_->WaitForGpuIdle(command_queue_.Get());

  if (!presentation_context_->Resize(width, height, command_queue_.Get(), frame_synchronizer_->GetFence())) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "PresentationContext resize failed");
    return false;
  }

  render_graph_->Resize(device_.Get(), width, height);

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

void Graphic::SetOverlayRenderer(OverlayRenderFunc renderer) {
  overlay_renderer_ = std::move(renderer);
  if (overlay_renderer_) {
    render_graph_->MarkExternallyReferenced(preview_handles_.scene_rt);
    render_graph_->MarkExternallyReferenced(preview_handles_.depth_preview_rt);
    render_graph_->MarkExternallyReferenced(preview_handles_.tonemap_rt);
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

      if (render_services_) {
        uint64_t completed = frame_synchronizer_->GetCompletedValue();
        render_services_->OnFrameBegin(0, completed);
        render_services_->OnFrameEnd();
      }
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

  if (render_graph_) {
    render_graph_->Shutdown();
  }

  render_services_.reset();
  presentation_context_.reset();

  is_initialized_ = false;
  is_shutting_down_.store(false, std::memory_order_release);
}

RenderFrameContext Graphic::BeginFrame() {
  uint32_t frame_index = presentation_context_->GetCurrentBackBufferIndex();

  if (debug_line_renderer_) {
    debug_line_renderer_->Clear();
  }

  frame_synchronizer_->WaitForFrame(frame_index);
  uint64_t completed_fence = frame_synchronizer_->GetCompletedValue();
  render_services_->OnFrameBegin(frame_index, completed_fence);

  object_cb_allocators_[frame_index]->Reset();

  command_context_->BeginFrame(frame_index);
  auto* cmd = command_context_->GetCommandList();

  descriptor_heap_manager_.BeginFrame(frame_index);
  descriptor_heap_manager_.SetDescriptorHeaps(cmd);

  render_graph_->BeginFrame();

  return RenderFrameContext{.frame_index = frame_index,
    .command_list = cmd,
    .frame_cb = &frame_cb_storage_.GetBuffer(frame_index),
    .object_cb_allocator = object_cb_allocators_[frame_index].get(),
    .dynamic_allocator = &descriptor_heap_manager_.GetSrvDynamicAllocator(frame_index),
    .global_heap_manager = &descriptor_heap_manager_,
    .screen_width = frame_buffer_width_,
    .screen_height = frame_buffer_height_,
    .render_graph = render_graph_.get()};
}

void Graphic::EndFrame(const RenderFrameContext& frame) {
  if (overlay_renderer_) {
    auto& swapchain = presentation_context_->GetSwapChainManager();
    auto rtv = swapchain.GetCurrentRTV();
    frame.command_list->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
    descriptor_heap_manager_.SetDescriptorHeaps(frame.command_list);
    overlay_renderer_(frame.command_list);
  }

  render_graph_->FinalizeFrame(frame.command_list);
  command_context_->Execute();

  uint64_t fence_value = frame_synchronizer_->SignalFrame(command_queue_.Get(), frame.frame_index);
  frame_cb_storage_.MarkFrameSubmitted(frame.frame_index, fence_value);

  render_services_->OnFrameEnd();

  presentation_context_->Present();
}

void Graphic::RenderScene(const RenderFrameContext& frame, const FramePacket& world) {
  render_graph_->Execute(frame, world);
}

void Graphic::AddDebugLine(const Vector3& start, const Vector3& end, const Vector4& color) {
  if (debug_line_renderer_) {
    debug_line_renderer_->AddLine(start, end, color);
  }
}

void Graphic::ExecuteSync(std::function<void(ID3D12GraphicsCommandList*)> cb) {
  command_context_->ExecuteSync(frame_synchronizer_->GetFence(), cb);
}
