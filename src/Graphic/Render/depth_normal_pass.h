#pragma once

#include <d3d12.h>

#include <vector>

#include "Core/types.h"
#include "Frame/draw_command.h"
#include "Frame/render_frame_context.h"
#include "Render/render_pass.h"

class ShaderManager;

class DepthNormalPass : public IRenderPass {
 public:
  struct Props {
    ID3D12Device* device;
    ShaderManager* shader_manager;
    PassSetup pass_setup;
  };

  explicit DepthNormalPass(const Props& props);

  const char* GetName() const override {
    return "Depth Normal Pass";
  }

  void Execute(const RenderFrameContext& frame, const FramePacket& packet) override;

 private:
  bool CreatePipelineState();

  ID3D12Device* device_;
  ShaderManager* shader_manager_;
  ComPtr<ID3D12PipelineState> pipeline_state_;
  std::vector<DrawCommand> grouped_commands_;
};
