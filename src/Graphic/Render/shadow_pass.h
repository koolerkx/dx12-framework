#pragma once

#include <d3d12.h>

#include <vector>

#include "Core/types.h"
#include "Frame/draw_command.h"
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
    uint32_t cascade_index = 0;
  };

  explicit ShadowPass(const Props& props);

  const char* GetName() const override {
    return name_buffer_;
  }

  void Execute(const RenderFrameContext& frame, const FramePacket& packet) override;

 private:
  bool CreatePipelineState();
  Math::Matrix4 ComputeLightViewProj(
    const CameraData& camera, const Math::Vector3& light_dir, float near_dist, float far_dist, float light_distance) const;

  ID3D12Device* device_;
  ShaderManager* shader_manager_;
  ShadowFrameData* shadow_data_;
  uint32_t cascade_index_;
  char name_buffer_[32];
  ComPtr<ID3D12PipelineState> pipeline_state_;
  std::vector<DrawCommand> grouped_commands_;
};
