#include "graphic.h"

#include <d3d12.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <dxgiformat.h>

#include <algorithm>
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
#include "Render/shadow_pass.h"
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

  for (int i = 0; i < FRAME_BUFFER_COUNT; ++i) {
    std::wstring name = L"PointLightBuffer_Frame" + std::to_wstring(i);
    if (!point_light_buffers_[i].Initialize(device_.Get(), MAX_POINT_LIGHTS, name)) {
      Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "Failed to initialize point light buffer {}", i);
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
  presentation_info.allow_tearing = device_context_->SupportsTearing();
  presentation_context_ = gfx::PresentationContext::Create(
    device_.Get(), dxgi_factory_.Get(), command_queue_.Get(), &descriptor_heap_manager_, presentation_info);
  if (!presentation_context_) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to create PresentationContext");
    return false;
  }

  render_graph_ = std::make_unique<RenderGraph>();
  render_graph_->SetSwapChain(&presentation_context_->GetSwapChainManager());
  render_graph_->SetHeapManager(&descriptor_heap_manager_);
  BuildRenderPipeline();

  is_initialized_ = true;
  return true;
}

void Graphic::BuildRenderPipeline() {
  auto backbuffer = render_graph_->ImportBackbuffer("backbuffer");

  auto scene_rt = render_graph_->CreateRenderTexture({
    .name = "scene_rt",
    .format = DXGI_FORMAT_R16G16B16A16_FLOAT,
    .width = frame_buffer_width_,
    .height = frame_buffer_height_,
    .device = device_.Get(),
    .clear_color = colors::DarkSlateGray,
  });

  auto scene_depth = render_graph_->CreateDepthBuffer({
    .name = "scene_depth",
    .width = frame_buffer_width_,
    .height = frame_buffer_height_,
    .device = device_.Get(),
  });

  auto tonemap_rt = render_graph_->CreateRenderTexture({
    .name = "tonemap_rt",
    .format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .width = frame_buffer_width_,
    .height = frame_buffer_height_,
    .device = device_.Get(),
  });

  auto depth_preview_rt = render_graph_->CreateRenderTexture({
    .name = "depth_preview_rt",
    .format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .width = frame_buffer_width_,
    .height = frame_buffer_height_,
    .device = device_.Get(),
  });

  static constexpr const char* SHADOW_DEPTH_NAMES[] = {"shadow_depth_0", "shadow_depth_1", "shadow_depth_2", "shadow_depth_3"};

  std::vector<RenderGraphHandle> shadow_reads;
  for (uint32_t i = 0; i < active_cascade_count_; ++i) {
    auto handle = render_graph_->CreateDepthBuffer({
      .name = SHADOW_DEPTH_NAMES[i],
      .width = shadow_frame_data_.shadow_map_resolution,
      .height = shadow_frame_data_.shadow_map_resolution,
      .device = device_.Get(),
      .fixed_size = true,
    });
    shadow_depth_handles_[i] = handle;
    shadow_reads.push_back(handle);

    PassSetup shadow_setup;
    shadow_setup.depth = handle;

    render_graph_->AddPass(std::make_unique<ShadowPass>(ShadowPass::Props{
      .device = device_.Get(),
      .shader_manager = &render_services_->GetShaderManager(),
      .pass_setup = shadow_setup,
      .shadow_data = &shadow_frame_data_,
      .cascade_index = i,
    }));
  }

  preview_handles_.scene_rt = scene_rt;
  preview_handles_.depth_preview_rt = depth_preview_rt;
  preview_handles_.tonemap_rt = tonemap_rt;
  preview_handles_.shadow_map_count = active_cascade_count_;
  for (uint32_t i = 0; i < active_cascade_count_; ++i) {
    preview_handles_.shadow_maps[i] = shadow_depth_handles_[i];
  }

  PassSetup scene_setup;
  scene_setup.resource_writes = {scene_rt};
  scene_setup.depth = scene_depth;
  scene_setup.resource_reads = shadow_reads;

  PassSetup backbuffer_setup;
  backbuffer_setup.resource_writes = {backbuffer};

  PassSetup tonemap_setup;
  tonemap_setup.resource_writes = {tonemap_rt};
  tonemap_setup.resource_reads = {scene_rt};

  render_graph_->AddPass(std::make_unique<SkyboxPass>(device_.Get(), &render_services_->GetShaderManager(), scene_setup));

  render_graph_->AddPass(std::make_unique<MaterialPass>(MaterialPass::MaterialPassProps{
    .name = "Opaque Pass",
    .renderer = opaque_renderer_.get(),
    .layer = RenderLayer::Opaque,
    .pass_setup = scene_setup,
  }));

  render_graph_->AddPass(std::make_unique<MaterialPass>(MaterialPass::MaterialPassProps{
    .name = "Transparent Pass",
    .renderer = transparent_renderer_.get(),
    .layer = RenderLayer::Transparent,
    .pass_setup = scene_setup,
  }));

  render_graph_->AddPass(std::make_unique<DebugPass>(debug_line_renderer_.get(), &render_services_->GetMaterialManager(), scene_setup));

  render_graph_->AddPass(std::make_unique<ToneMapPass>(ToneMapPassProps{
    .device = device_.Get(),
    .material_manager = &render_services_->GetMaterialManager(),
    .shader_manager = &render_services_->GetShaderManager(),
    .pass_setup = tonemap_setup,
    .hdr_handle = scene_rt,
    .debug = &hdr_debug_,
  }));

  PassSetup depth_preview_setup;
  depth_preview_setup.resource_writes = {depth_preview_rt};
  depth_preview_setup.resource_reads = {scene_depth};

  render_graph_->AddPass(std::make_unique<DepthViewPass>(DepthViewPassProps{
    .device = device_.Get(),
    .shader_manager = &render_services_->GetShaderManager(),
    .pass_setup = depth_preview_setup,
    .depth_handle = scene_depth,
    .config = &depth_preview_config_,
  }));

  PassSetup blit_setup;
  blit_setup.resource_writes = {backbuffer};
  blit_setup.resource_reads = {tonemap_rt};

  render_graph_->AddPass(std::make_unique<BlitPass>(BlitPassProps{
    .device = device_.Get(),
    .shader_manager = &render_services_->GetShaderManager(),
    .pass_setup = blit_setup,
    .source_handle = tonemap_rt,
  }));

  PassSetup depth_view_setup;
  depth_view_setup.resource_writes = {backbuffer};
  depth_view_setup.resource_reads = {scene_depth};

  render_graph_->AddPass(std::make_unique<DepthViewPass>(DepthViewPassProps{
    .device = device_.Get(),
    .shader_manager = &render_services_->GetShaderManager(),
    .pass_setup = depth_view_setup,
    .depth_handle = scene_depth,
    .config = &depth_view_config_,
  }));

  auto ui_camera_from_packet = [](const RenderFrameContext&, const FramePacket& packet) { return packet.ui_camera; };
  render_graph_->AddPass(std::make_unique<MaterialPass>(MaterialPass::MaterialPassProps{
    .name = "UI Pass",
    .renderer = ui_renderer_.get(),
    .layer = RenderLayer::UI,
    .pass_setup = backbuffer_setup,
    .camera = ui_camera_from_packet,
  }));
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
    for (uint32_t i = 0; i < preview_handles_.shadow_map_count; ++i) {
      render_graph_->MarkExternallyReferenced(preview_handles_.shadow_maps[i]);
    }
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

  mesh_registry_.Clear();
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

  shadow_frame_data_.cascade_count = active_cascade_count_;
  for (uint32_t i = 0; i < active_cascade_count_; ++i) {
    shadow_frame_data_.shadow_map_srv_index[i] = render_graph_->GetSrvIndex(shadow_depth_handles_[i]);
  }

  return RenderFrameContext{.frame_index = frame_index,
    .command_list = cmd,
    .frame_cb = &frame_cb_storage_.GetBuffer(frame_index),
    .object_cb_allocator = object_cb_allocators_[frame_index].get(),
    .dynamic_allocator = &descriptor_heap_manager_.GetSrvDynamicAllocator(frame_index),
    .global_heap_manager = &descriptor_heap_manager_,
    .screen_width = frame_buffer_width_,
    .screen_height = frame_buffer_height_,
    .render_graph = render_graph_.get(),
    .shadow_data = &shadow_frame_data_,
    .point_light_srv = point_light_buffers_[frame_index].GetGPUAddress()};
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

void Graphic::UploadPointLights(RenderFrameContext& frame, const FramePacket& world) {
  uint32_t light_count = static_cast<uint32_t>(world.point_lights.size());
  if (light_count > 0) {
    std::vector<PointLightData> gpu_lights(light_count);
    for (uint32_t i = 0; i < light_count; ++i) {
      const auto& src = world.point_lights[i];
      gpu_lights[i].position = src.position;
      gpu_lights[i].intensity = src.intensity;
      gpu_lights[i].color = src.color;
      gpu_lights[i].radius = src.radius;
      gpu_lights[i].falloff = src.falloff;
      gpu_lights[i].enabled = 1;
    }
    point_light_buffers_[frame.frame_index].Update(gpu_lights);
  }
  frame.point_light_count = light_count;
}

void Graphic::RenderScene(const RenderFrameContext& frame, const FramePacket& world) {
  render_graph_->Execute(frame, world);
}

void Graphic::SetShadowMapResolution(uint32_t resolution) {
  if (shadow_frame_data_.shadow_map_resolution == resolution) return;
  shadow_frame_data_.shadow_map_resolution = resolution;
  for (uint32_t i = 0; i < active_cascade_count_; ++i) {
    render_graph_->ResizeDepthBuffer(shadow_depth_handles_[i], device_.Get(), resolution, resolution);
    shadow_frame_data_.shadow_map_srv_index[i] = render_graph_->GetSrvIndex(shadow_depth_handles_[i]);
  }
}

void Graphic::SetCascadeCount(uint32_t count) {
  count = std::clamp(count, 1u, ShadowCascadeConfig::MAX_CASCADES);
  if (count == active_cascade_count_) return;
  WaitForGpuIdle();
  active_cascade_count_ = count;
  render_graph_ = std::make_unique<RenderGraph>();
  render_graph_->SetSwapChain(&presentation_context_->GetSwapChainManager());
  render_graph_->SetHeapManager(&descriptor_heap_manager_);
  BuildRenderPipeline();
  if (overlay_renderer_) SetOverlayRenderer(std::move(overlay_renderer_));
}

void Graphic::AddDebugLine(const Vector3& start, const Vector3& end, const Vector4& color) {
  if (debug_line_renderer_) {
    debug_line_renderer_->AddLine(start, end, color);
  }
}

void Graphic::ExecuteSync(std::function<void(ID3D12GraphicsCommandList*)> cb) {
  command_context_->ExecuteSync(frame_synchronizer_->GetFence(), cb);
}
