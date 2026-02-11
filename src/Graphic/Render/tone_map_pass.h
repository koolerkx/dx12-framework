#pragma once

#include <d3d12.h>

#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"
#include "Render/render_graph_handle.h"
#include "Render/render_pass.h"
#include "Rendering/hdr_config.h"

struct ID3D12Device;
class MaterialManager;
class ShaderManager;

struct ToneMapPassProps {
  ID3D12Device* device;
  MaterialManager* material_manager;
  ShaderManager* shader_manager;
  PassSetup pass_setup;
  RenderGraphHandle hdr_handle;
  const HdrDebug* debug;
};

class ToneMapPass : public IRenderPass {
 public:
  explicit ToneMapPass(const ToneMapPassProps& props);
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

  RenderGraphHandle hdr_handle_ = RenderGraphHandle::Invalid;
  const HdrDebug* debug_ = nullptr;
};
