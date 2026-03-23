#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

#include <memory>
#include <vector>

#include "Command/command_context.h"
#include "Core/types.h"
#include "Debug/debug_line_renderer.h"
#include "Descriptor/descriptor_heap_manager.h"
#include "Device/device_context.h"
#include "Device/fence_manager.h"
#include "Device/frame_synchronizer.h"
#include "Frame/constant_buffers.h"
#include "Frame/dynamic_upload_buffer.h"
#include "Framework/Render/frame_packet.h"
#include "Frame/per_frame_constant_buffer.h"
#include "Frame/render_frame_context.h"
#include "Pipeline/material_manager.h"
#include "Pipeline/shader_manager.h"
#include "Presentation/presentation_context.h"
#include "Render/material_renderer.h"
#include "Render/render_graph.h"
#include "Render/render_graph_handle.h"
#include "Rendering/chromatic_aberration_config.h"
#include "Rendering/fog_config.h"
#include "Rendering/hdr_config.h"
#include "Rendering/outline_config.h"
#include "Rendering/smaa_config.h"
#include "Rendering/ssao_config.h"
#include "Rendering/vignette_config.h"
#include "Resource/Buffer/structured_buffer.h"
#include "Resource/Font/sprite_font_manager.h"
#include "Resource/Material/material_descriptor_pool.h"
#include "Resource/Mesh/mesh_buffer_pool.h"
#include "Resource/Texture/texture_manager.h"
#include "Resource/mesh_registry.h"
#include "Resource/render_services.h"

class Graphic {
 public:
  struct RenderConfig {
    HdrDebug hdr_debug;
    DepthViewConfig depth_view;
    DepthViewConfig depth_preview = {.enabled = true, .near_plane = 0.1f, .far_plane = 1000.0f};
    BloomConfig bloom;
    SSAOConfig ssao;
    SMAAConfig smaa;
    FogConfig fog;
    OutlineConfig outline;
    VignetteConfig vignette;
    ChromaticAberrationConfig chromatic_aberration;
  };

  struct GraphicInitProps {
    bool enable_vsync = true;
    BloomConfig bloom;
    SSAOConfig ssao;
    SMAAConfig smaa;
    FogConfig fog;
    OutlineConfig outline;
    VignetteConfig vignette;
  };

  Graphic() = default;
  ~Graphic() {
    Shutdown();
  };

  bool Initialize(HWND hwnd, UINT frame_buffer_width, UINT frame_buffer_height, const GraphicInitProps& props);
  void Shutdown();

  void WaitForGpuIdle();

  bool ResizeBuffers(UINT width, UINT height);

  void ExecuteSync(std::function<void(ID3D12GraphicsCommandList*)> cb);

  RenderFrameContext BeginFrame();
  void EndFrame(const RenderFrameContext& frame);
  void RenderScene(RenderFrameContext& frame, const FramePacket& world);

  void AddDebugLine(const Math::Vector3& start, const Math::Vector3& end, const Math::Vector4& color = colors::White);

  DebugLineRenderer* GetDebugLineRenderer() {
    return debug_line_renderer_.get();
  }

  static constexpr uint32_t FRAME_BUFFER_COUNT = 2;

  TextureManager& GetTextureManager() {
    return render_services_->GetTextureManager();
  }

  MaterialManager& GetMaterialManager() {
    return render_services_->GetMaterialManager();
  }

  ShaderManager& GetShaderManager() {
    return render_services_->GetShaderManager();
  }

  FenceManager& GetFenceManager() {
    return frame_synchronizer_->GetFenceManager();
  }

  Font::SpriteFontManager& GetSpriteFontManager() {
    return render_services_->GetFontManager();
  }

  MeshBufferPool& GetMeshBufferPool() {
    return render_services_->GetMeshBufferPool();
  }

  MaterialDescriptorPool& GetMaterialDescriptorPool() {
    return render_services_->GetMaterialDescriptorPool();
  }

  MeshRegistry& GetMeshRegistry() {
    return mesh_registry_;
  }

  ID3D12Device* GetDevice() const {
    return device_context_ ? device_context_->GetDevice() : nullptr;
  }

  ID3D12CommandQueue* GetCommandQueue() const {
    return command_queue_.Get();
  }

  void SetVSync(bool enable) {
    enable_vsync_ = enable;
    if (presentation_context_) {
      presentation_context_->SetVSync(enable);
    }
  }

  bool IsVSyncEnabled() const {
    return enable_vsync_;
  }

  UINT GetFrameBufferWidth() const {
    return frame_buffer_width_;
  }
  UINT GetFrameBufferHeight() const {
    return frame_buffer_height_;
  }

  uint32_t GetSceneWidth() const {
    return scene_width_;
  }
  uint32_t GetSceneHeight() const {
    return scene_height_;
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

  uint64_t GetCurrentFenceValue() const {
    return frame_synchronizer_ ? frame_synchronizer_->GetCurrentFenceValue() : 0;
  }

  struct RenderTargetPreview {
    RenderGraphHandle scene_rt = RenderGraphHandle::Invalid;
    RenderGraphHandle depth_preview_rt = RenderGraphHandle::Invalid;
    RenderGraphHandle tonemap_rt = RenderGraphHandle::Invalid;
    RenderGraphHandle normal_depth_rt = RenderGraphHandle::Invalid;
    RenderGraphHandle normal_preview_rt = RenderGraphHandle::Invalid;
    RenderGraphHandle linear_depth_preview_rt = RenderGraphHandle::Invalid;
    RenderGraphHandle ssao_rt = RenderGraphHandle::Invalid;
    RenderGraphHandle viewport_rt = RenderGraphHandle::Invalid;
    RenderGraphHandle shadow_maps[ShadowCascadeConfig::MAX_CASCADES];
    uint32_t shadow_map_count = 0;
  };

  uint32_t GetNormalDepthSrvIndex() const;
  uint32_t GetUIBlurSrvIndex() const;

  RenderGraph* GetRenderGraph() {
    return render_graph_.get();
  }

  const RenderTargetPreview& GetPreviewHandles() const {
    return preview_handles_;
  }

  HdrDebug& GetHdrDebug() {
    return render_config_.hdr_debug;
  }
  DepthViewConfig& GetDepthViewConfig() {
    return render_config_.depth_view;
  }
  DepthViewConfig& GetDepthPreviewConfig() {
    return render_config_.depth_preview;
  }
  BloomConfig& GetBloomConfig() {
    return render_config_.bloom;
  }
  SSAOConfig& GetSSAOConfig() {
    return render_config_.ssao;
  }
  SMAAConfig& GetSMAAConfig() {
    return render_config_.smaa;
  }
  OutlineConfig& GetOutlineConfig() {
    return render_config_.outline;
  }
  FogConfig& GetFogConfig() {
    return render_config_.fog;
  }
  VignetteConfig& GetVignetteConfig() {
    return render_config_.vignette;
  }
  ChromaticAberrationConfig& GetChromaticAberrationConfig() {
    return render_config_.chromatic_aberration;
  }

  void SetWireframeMode(bool enabled) {
    if (render_services_) render_services_->GetMaterialManager().SetWireframeOverride(enabled);
  }
  bool IsWireframeMode() {
    return render_services_ ? render_services_->GetMaterialManager().IsWireframeOverride() : false;
  }

  using OverlayRenderFunc = std::function<void(ID3D12GraphicsCommandList*)>;
  void SetOverlayRenderer(OverlayRenderFunc renderer);

 private:
  std::unique_ptr<gfx::DeviceContext> device_context_;
  std::unique_ptr<gfx::CommandContext> command_context_;
  std::unique_ptr<gfx::PresentationContext> presentation_context_;
  std::unique_ptr<gfx::FrameSynchronizer> frame_synchronizer_;

  ComPtr<ID3D12Device5> device_ = nullptr;
  ComPtr<IDXGIFactory6> dxgi_factory_ = nullptr;
  ComPtr<ID3D12CommandQueue> command_queue_ = nullptr;

  DescriptorHeapManager descriptor_heap_manager_;
  MeshRegistry mesh_registry_;

  RenderConfig render_config_;
  RenderGraphHandle ssao_handle_ = RenderGraphHandle::Invalid;
  RenderGraphHandle ui_blur_rt_ = RenderGraphHandle::Invalid;
  RenderGraphHandle viewport_rt_ = RenderGraphHandle::Invalid;
  bool pending_pipeline_rebuild_ = false;

  ShadowFrameData shadow_frame_data_;
  RenderGraphHandle shadow_depth_handles_[ShadowCascadeConfig::MAX_CASCADES];
  uint32_t active_cascade_count_ = ShadowCascadeConfig::DEFAULT_CASCADE_COUNT;

  UINT frame_buffer_width_ = 0;
  UINT frame_buffer_height_ = 0;
  uint32_t scene_width_ = 0;
  uint32_t scene_height_ = 0;

  PerFrameConstantBuffer<FrameCB> frame_cb_storage_;
  std::vector<std::unique_ptr<DynamicUploadBuffer>> object_cb_allocators_;

  // TODO: Abstract as UplaodStructuredBuffer when more structured buffers are needed
  Graphics::StructuredBuffer<PointLightData> point_light_buffers_[FRAME_BUFFER_COUNT];

  std::unique_ptr<gfx::RenderServices> render_services_;

  std::unique_ptr<UiRenderer> ui_renderer_;
  std::unique_ptr<OpaqueRenderer> opaque_renderer_;
  std::unique_ptr<TransparentRenderer> transparent_renderer_;
  std::unique_ptr<RenderGraph> render_graph_;

  std::unique_ptr<DebugLineRenderer> debug_line_renderer_;
  RenderTargetPreview preview_handles_;

  HWND hwnd_ = nullptr;
  bool enable_vsync_ = true;
  OverlayRenderFunc overlay_renderer_;
  bool preview_pipeline_active_ = true;
  bool preview_shadow_active_ = false;

  void BuildRenderPipeline();
  void MarkActivePreviewResources();
  void UploadPointLights(RenderFrameContext& frame, const FramePacket& world);

 public:
  void RequestPipelineRebuild();
  void SetPreviewPipelineActive(bool active);
  void SetPreviewShadowActive(bool active);
  void SetShadowMapResolution(uint32_t resolution);
  void SetCascadeCount(uint32_t count);
  const ShadowFrameData& GetShadowFrameData() const {
    return shadow_frame_data_;
  }

 private:
  bool is_initialized_ = false;
  std::atomic<bool> is_shutting_down_{false};
};
