#include "ui_blur_pass_group.h"

#include <algorithm>

#include "Pipeline/shader_descriptors.h"
#include "Render/fullscreen_pass.h"
#include "Render/render_graph.h"

namespace {

struct UIBlurCB {
  uint32_t source_srv_index;
  float texel_size_x;
  float texel_size_y;
  float threshold;
};
static_assert(sizeof(UIBlurCB) == 16);

class UIBlurDownsamplePass : public FullscreenPass<Graphics::PostProcessBloomDownsampleShader> {
 public:
  UIBlurDownsamplePass(
    ID3D12Device* device, ShaderManager* shader_manager, const PassSetup& pass_setup, RenderGraphHandle source_handle, const char* name)
      : FullscreenPass(device, shader_manager, pass_setup, {.rt_format = DXGI_FORMAT_R11G11B10_FLOAT, .pso_name = L"UIBlurDown_PSO"}),
        source_handle_(source_handle),
        name_(name) {
  }

  const char* GetName() const override {
    return name_;
  }

 protected:
  void SetupConstants(RenderCommandList& cmd, const RenderFrameContext& frame, const FramePacket&) override {
    auto [src_w, src_h] = frame.render_graph->GetTextureSize(source_handle_);

    UIBlurCB cb_data{
      .source_srv_index = frame.render_graph->GetSrvIndex(source_handle_),
      .texel_size_x = 1.0f / static_cast<float>(src_w),
      .texel_size_y = 1.0f / static_cast<float>(src_h),
      .threshold = 0.0f,
    };

    constexpr auto POST_PROCESS_CB = RootSlot::ConstantBuffer::Light;
    cmd.SetConstantBufferOverride(POST_PROCESS_CB, cb_data);
  }

 private:
  RenderGraphHandle source_handle_;
  const char* name_;
};

struct UIBlurUpCB {
  uint32_t source_srv_index;
  float texel_size_x;
  float texel_size_y;
  float padding;
};
static_assert(sizeof(UIBlurUpCB) == 16);

class UIBlurUpsamplePass : public FullscreenPass<Graphics::PostProcessBloomUpsampleShader> {
 public:
  UIBlurUpsamplePass(
    ID3D12Device* device, ShaderManager* shader_manager, const PassSetup& pass_setup, RenderGraphHandle source_handle, const char* name)
      : FullscreenPass(device,
          shader_manager,
          pass_setup,
          {.rt_format = DXGI_FORMAT_R11G11B10_FLOAT, .blend_mode = BlendMode::Opaque, .pso_name = L"UIBlurUp_PSO"}),
        source_handle_(source_handle),
        name_(name) {
  }

  const char* GetName() const override {
    return name_;
  }

 protected:
  void SetupConstants(RenderCommandList& cmd, const RenderFrameContext& frame, const FramePacket&) override {
    auto [src_w, src_h] = frame.render_graph->GetTextureSize(source_handle_);

    UIBlurUpCB cb_data{
      .source_srv_index = frame.render_graph->GetSrvIndex(source_handle_),
      .texel_size_x = 1.0f / static_cast<float>(src_w),
      .texel_size_y = 1.0f / static_cast<float>(src_h),
      .padding = 0.0f,
    };

    constexpr auto POST_PROCESS_CB = RootSlot::ConstantBuffer::Light;
    cmd.SetConstantBufferOverride(POST_PROCESS_CB, cb_data);
  }

 private:
  RenderGraphHandle source_handle_;
  const char* name_;
};

}  // namespace

void UIBlurPassGroup::Build(RenderGraph& graph, RenderGraphHandle scene_source, const UIBlurBuildProps& props) {
  static constexpr uint32_t DOWN_COUNT = 3;
  static constexpr uint32_t UP_COUNT = 2;

  RenderGraphHandle down_handles[DOWN_COUNT];

  static constexpr const char* DOWN_RT_NAMES[] = {"ui_blur_down_0", "ui_blur_down_1", "ui_blur_down_2"};
  static constexpr const char* DOWN_PASS_NAMES[] = {"UI Blur Down 0", "UI Blur Down 1", "UI Blur Down 2"};

  for (uint32_t i = 0; i < DOWN_COUNT; ++i) {
    float scale = 1.0f / static_cast<float>(1u << (i + 1));
    uint32_t w = (std::max)(static_cast<uint32_t>(props.screen_width * scale), 1u);
    uint32_t h = (std::max)(static_cast<uint32_t>(props.screen_height * scale), 1u);

    down_handles[i] = graph.CreateRenderTexture({
      .name = DOWN_RT_NAMES[i],
      .format = DXGI_FORMAT_R11G11B10_FLOAT,
      .width = w,
      .height = h,
      .device = props.device,
      .scale_factor = scale,
    });
  }

  for (uint32_t i = 0; i < DOWN_COUNT; ++i) {
    RenderGraphHandle source = (i == 0) ? scene_source : down_handles[i - 1];

    PassSetup setup;
    setup.resource_writes = {down_handles[i]};
    setup.resource_reads = {source};

    AddPass(graph, std::make_unique<UIBlurDownsamplePass>(props.device, props.shader_manager, setup, source, DOWN_PASS_NAMES[i]));
  }

  static constexpr const char* UP_RT_NAMES[] = {"ui_blur_up_0", "ui_blur_up_1"};
  static constexpr const char* UP_PASS_NAMES[] = {"UI Blur Up 0", "UI Blur Up 1"};

  RenderGraphHandle prev = down_handles[DOWN_COUNT - 1];

  for (uint32_t i = 0; i < UP_COUNT; ++i) {
    uint32_t target_mip = DOWN_COUNT - 2 - i;
    float scale = 1.0f / static_cast<float>(1u << (target_mip + 1));
    uint32_t w = (std::max)(static_cast<uint32_t>(props.screen_width * scale), 1u);
    uint32_t h = (std::max)(static_cast<uint32_t>(props.screen_height * scale), 1u);

    RenderGraphHandle up_rt = graph.CreateRenderTexture({
      .name = UP_RT_NAMES[i],
      .format = DXGI_FORMAT_R11G11B10_FLOAT,
      .width = w,
      .height = h,
      .device = props.device,
      .scale_factor = scale,
    });

    PassSetup setup;
    setup.resource_writes = {up_rt};
    setup.resource_reads = {prev};

    AddPass(graph, std::make_unique<UIBlurUpsamplePass>(props.device, props.shader_manager, setup, prev, UP_PASS_NAMES[i]));

    prev = up_rt;
  }

  blurred_output_ = prev;
}
