#pragma once

#include <d3d12.h>

#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"
#include "Render/render_pass.h"
#include "Rendering/hdr_config.h"

struct ID3D12Device;
class MaterialManager;
class ShaderManager;
class RenderTexture;

class ToneMapPass : public IRenderPass {
 public:
  ToneMapPass(ID3D12Device* device,
    MaterialManager* material_manager,
    ShaderManager* shader_manager,
    PassSetup pass_setup,
    RenderTexture* hdr_texture,
    const HdrConfig* config,
    const HdrDebug* debug);
  ~ToneMapPass() = default;

  const char* GetName() const override {
    return "Tone Mapping Pass";
  }

  void Execute(const RenderFrameContext& frame, const FramePacket& packet) override;

 private:
  bool CreatePipelineObjects();

  ID3D12Device* device_;
  MaterialManager* material_manager_;
  ShaderManager* shader_manager_;
  ComPtr<ID3D12PipelineState> pipeline_state_;

  RenderTexture* hdr_texture_ = nullptr;
  const HdrConfig* config_ = nullptr;
  const HdrDebug* debug_ = nullptr;
};
