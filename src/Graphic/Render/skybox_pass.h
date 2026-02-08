#pragma once

#include <d3d12.h>

#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"
#include "Render/render_pass.h"
#include "Resource/mesh.h"

struct ID3D12Device;
class ShaderManager;

class SkyboxPass : public IRenderPass {
 public:
  SkyboxPass(ID3D12Device* device, ShaderManager* shader_manager, PassSetup pass_setup);

  const char* GetName() const override {
    return "Skybox Pass";
  }

  void Execute(const RenderFrameContext& frame, const FramePacket& packet) override;

 private:
  bool CreatePipelineObjects();
  bool CreateCubeMesh();

  ID3D12Device* device_;
  ShaderManager* shader_manager_;
  ComPtr<ID3D12PipelineState> pipeline_state_;
  Mesh cube_mesh_;
};
