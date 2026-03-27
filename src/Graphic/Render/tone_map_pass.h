#pragma once

#include "Pipeline/shader_descriptors.h"
#include "Render/fullscreen_pass.h"
#include "Render/render_graph.h"
#include "Rendering/post_process_config.h"

class MaterialManager;
struct BloomConfig;

struct ToneMapPassProps {
  ID3D12Device* device;
  MaterialManager* material_manager;
  ShaderManager* shader_manager;
  PassSetup pass_setup;
  RenderGraphHandle hdr_handle;
  const HdrDebug* debug;
  RenderGraphHandle bloom_handle = RenderGraphHandle::Invalid;
  const BloomConfig* bloom_config = nullptr;
};

class ToneMapPass : public FullscreenPass<Graphics::PostProcessToneMapShader> {
 public:
  explicit ToneMapPass(const ToneMapPassProps& props)
      : FullscreenPass(props.device, props.shader_manager, props.pass_setup, {.pso_name = L"ToneMapPass_PSO"}),
        hdr_handle_(props.hdr_handle),
        bloom_handle_(props.bloom_handle),
        debug_(props.debug),
        bloom_config_(props.bloom_config) {
  }

  const char* GetName() const override {
    return "Tone Mapping Pass";
  }
  const wchar_t* GetWideName() const override {
    return L"Tone Mapping Pass";
  }

 protected:
  void SetupConstants(RenderCommandList& cmd, const RenderFrameContext& frame, const FramePacket& packet) override {
    struct ToneMapCB {
      float exposure;
      uint32_t debug_mode;
      uint32_t hdr_srv_index;
      uint32_t bloom_srv_index;
      float bloom_intensity;
      uint32_t padding[3] = {};
    };
    static_assert(sizeof(ToneMapCB) == 32);

    uint32_t bloom_srv = 0xFFFFFFFF;
    float bloom_intensity = 0.0f;
    if (bloom_handle_ != RenderGraphHandle::Invalid && bloom_config_) {
      bloom_srv = frame.render_graph->GetSrvIndex(bloom_handle_);
      bloom_intensity = bloom_config_->intensity;
    }

    ToneMapCB cb_data = {
      .exposure = packet.main_camera.exposure,
      .debug_mode = debug_->debug_view ? 1u : 0u,
      .hdr_srv_index = frame.render_graph->GetSrvIndex(hdr_handle_),
      .bloom_srv_index = bloom_srv,
      .bloom_intensity = bloom_intensity,
    };

    constexpr auto POST_PROCESS_CB = RootSlot::ConstantBuffer::Light;
    cmd.SetConstantBufferOverride(POST_PROCESS_CB, cb_data);
  }

 private:
  RenderGraphHandle hdr_handle_;
  RenderGraphHandle bloom_handle_;
  const HdrDebug* debug_;
  const BloomConfig* bloom_config_;
};
