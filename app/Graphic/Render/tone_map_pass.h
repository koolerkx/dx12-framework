#pragma once
#include <d3d12.h>

#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"
#include "Render/render_pass.h"
#include "Rendering/hdr_config.h"

class HdrRenderTarget;
class SwapChainManager;
struct ID3D12Device;
class MaterialManager;
class ShaderManager;

class ToneMapPass : public IRenderPass {
 public:
  ToneMapPass(ID3D12Device* device, MaterialManager* material_manager, ShaderManager* shader_manager);
  ~ToneMapPass() = default;

  // Set dependencies (must be called before adding to RenderPassManager)
  void SetDependencies(HdrRenderTarget* hdr_rt, SwapChainManager* swapchain_manager, const HdrConfig* config, const HdrDebug* debug) {
    hdr_rt_ = hdr_rt;
    swapchain_manager_ = swapchain_manager;
    config_ = config;
    debug_ = debug;
  }

  const char* GetName() const override {
    return "Tone Mapping Pass";
  }

  // Standard IRenderPass interface - called by RenderPassManager
  void PreExecute(const RenderFrameContext& frame, const FramePacket& packet) override;
  void Execute(const RenderFrameContext& frame, const FramePacket& packet) override;

 private:
  bool CreatePipelineObjects();

  ID3D12Device* device_;
  MaterialManager* material_manager_;
  ShaderManager* shader_manager_;
  ComPtr<ID3D12PipelineState> pipeline_state_;

  // Dependencies (set via SetDependencies)
  HdrRenderTarget* hdr_rt_ = nullptr;
  SwapChainManager* swapchain_manager_ = nullptr;
  const HdrConfig* config_ = nullptr;
  const HdrDebug* debug_ = nullptr;
};
