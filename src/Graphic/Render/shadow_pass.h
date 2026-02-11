#pragma once

#include <d3d12.h>

#include "Core/types.h"
#include "Frame/render_frame_context.h"
#include "Render/render_pass.h"

class ShaderManager;

class ShadowPass : public IRenderPass {
 public:
  struct Props {
    ID3D12Device* device;
    ShaderManager* shader_manager;
    PassSetup pass_setup;
    ShadowFrameData* shadow_data;
  };

  explicit ShadowPass(const Props& props);

  const char* GetName() const override {
    return "Shadow Pass";
  }

  void Execute(const RenderFrameContext& frame, const FramePacket& packet) override;

 private:
  bool CreatePipelineState();
  Math::Matrix4 ComputeLightViewProj(const CameraData& camera, const Math::Vector3& light_dir, float shadow_distance) const;

  ID3D12Device* device_;
  ShaderManager* shader_manager_;
  ShadowFrameData* shadow_data_;
  ComPtr<ID3D12PipelineState> pipeline_state_;
};
