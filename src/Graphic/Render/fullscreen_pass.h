#pragma once

#include <d3d12.h>

#include "Command/render_command_list.h"
#include "Core/types.h"
#include "Framework/Logging/logger.h"
#include "Pipeline/pipeline_state_builder.h"
#include "Pipeline/shader_manager.h"
#include "Render/render_pass.h"

struct FullscreenPassConfig {
  DXGI_FORMAT rt_format = DXGI_FORMAT_R8G8B8A8_UNORM;
  BlendMode blend_mode = BlendMode::Opaque;
  const wchar_t* pso_name = L"Fullscreen_PSO";
};

template <typename ShaderType>
class FullscreenPass : public IRenderPass {
 public:
  FullscreenPass(ID3D12Device* device, ShaderManager* shader_manager, const PassSetup& pass_setup, const FullscreenPassConfig& config)
      : device_(device), shader_manager_(shader_manager) {
    setup_ = pass_setup;
    CreatePSO(config);
  }

 protected:
  virtual void SetupConstants(RenderCommandList& cmd, const RenderFrameContext& frame, const FramePacket& packet) = 0;

  virtual bool ShouldExecute(const RenderFrameContext&, const FramePacket&) const {
    return pipeline_state_ != nullptr;
  }

  void Execute(const RenderFrameContext& frame, const FramePacket& packet) override {
    if (!ShouldExecute(frame, packet)) return;

    RenderCommandList cmd(frame.command_list, frame.dynamic_allocator, frame.object_cb_allocator);

    auto* root_sig = shader_manager_->GetRootSignature(Graphics::RSPreset::Standard);
    frame.command_list->SetGraphicsRootSignature(root_sig);
    frame.command_list->SetPipelineState(pipeline_state_.Get());
    cmd.BindGlobalSRVTable(frame.global_heap_manager);
    cmd.BindSamplerTable(frame.global_heap_manager);

    SetupConstants(cmd, frame, packet);

    frame.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    frame.command_list->DrawInstanced(3, 1, 0, 0);
  }

  ID3D12Device* device_;
  ShaderManager* shader_manager_;
  ComPtr<ID3D12PipelineState> pipeline_state_;

 private:
  void CreatePSO(const FullscreenPassConfig& config) {
    auto* vs = shader_manager_->GetVertexShader<ShaderType>();
    auto* ps = shader_manager_->GetPixelShader<ShaderType>();
    auto* root_sig = shader_manager_->GetRootSignature(Graphics::RSPreset::Standard);
    if (!vs || !ps || !root_sig) {
      Logger::Log(LogLevel::Error, LogCategory::Graphic, "[FullscreenPass] Shader/root signature load failed", Logger::Here());
      return;
    }
    pipeline_state_ = PipelineStateBuilder()
                        .SetRootSignature(root_sig)
                        .SetVertexShader(vs)
                        .SetPixelShader(ps)
                        .SetRenderTargetFormat(config.rt_format)
                        .SetBlendMode(config.blend_mode)
                        .SetName(config.pso_name)
                        .Build(device_);
  }
};
