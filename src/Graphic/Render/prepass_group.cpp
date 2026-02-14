#include "prepass_group.h"

#include "Render/depth_normal_pass.h"
#include "Render/render_graph.h"

void PrepassGroup::Build(RenderGraph& graph, const Props& props) {
  normal_depth_rt_ = graph.CreateRenderTexture({
    .name = "normal_depth_rt",
    .format = DXGI_FORMAT_R16G16B16A16_FLOAT,
    .width = props.context.width,
    .height = props.context.height,
    .device = props.context.device,
  });

  prepass_depth_ = graph.CreateDepthBuffer({
    .name = "prepass_depth",
    .width = props.context.width,
    .height = props.context.height,
    .device = props.context.device,
  });

  PassSetup depth_normal_setup;
  depth_normal_setup.resource_writes = {normal_depth_rt_};
  depth_normal_setup.depth = prepass_depth_;

  AddPass(graph, std::make_unique<DepthNormalPass>(DepthNormalPass::Props{
    .device = props.context.device,
    .shader_manager = props.context.shader_manager,
    .pass_setup = depth_normal_setup,
  }));
}
