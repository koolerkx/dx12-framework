#pragma once
#include <d3d12.h>

#include "Frame/draw_command.h"
#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"
#include "Presentation/presentation_context.h"
#include "Render/render_pass.h"
#include "Rendering/hdr_config.h"

struct ID3D12Device;
class MaterialManager;
class ShaderManager;

class ToneMapPass : public IRenderPass {
 public:
  ToneMapPass(ID3D12Device* device, MaterialManager* material_manager, ShaderManager* shader_manager);
  ~ToneMapPass() = default;

  // Set dependencies (must be called before adding to RenderPassManager)
  void SetDependencies(gfx::PresentationContext* presentation_context, const HdrConfig* config, const HdrDebug* debug) {
    presentation_context_ = presentation_context;
    config_ = config;
    debug_ = debug;
  }

  const char* GetName() const override {
    return "Tone Mapping Pass";
  }

  RenderPassFilter GetFilter() const override {
    // ToneMapPass is a post-process pass, doesn't use the unified command system
    return RenderPassFilter{.target_layer = RenderLayer::Debug, .required_tags = UINT32_MAX, .excluded_tags = 0};
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
  gfx::PresentationContext* presentation_context_ = nullptr;
  const HdrConfig* config_ = nullptr;
  const HdrDebug* debug_ = nullptr;
};
