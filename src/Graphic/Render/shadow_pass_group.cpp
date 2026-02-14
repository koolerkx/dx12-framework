#include "shadow_pass_group.h"

#include "Render/render_graph.h"
#include "Render/shadow_pass.h"

void ShadowPassGroup::Build(RenderGraph& graph, const Props& props) {
  static constexpr const char* SHADOW_DEPTH_NAMES[] = {
    "shadow_depth_0", "shadow_depth_1", "shadow_depth_2", "shadow_depth_3"};

  shadow_maps_.clear();
  shadow_maps_.reserve(props.cascade_count);

  for (uint32_t i = 0; i < props.cascade_count; ++i) {
    auto handle = graph.CreateDepthBuffer({
      .name = SHADOW_DEPTH_NAMES[i],
      .width = props.resolution,
      .height = props.resolution,
      .device = props.context.device,
      .fixed_size = true,
    });

    props.shadow_depth_handles[i] = handle;
    shadow_maps_.push_back(handle);

    PassSetup shadow_setup;
    shadow_setup.depth = handle;

    AddPass(graph, std::make_unique<ShadowPass>(ShadowPass::Props{
      .device = props.context.device,
      .shader_manager = props.context.shader_manager,
      .pass_setup = shadow_setup,
      .shadow_data = props.shadow_data,
      .cascade_index = i,
    }));
  }
}
