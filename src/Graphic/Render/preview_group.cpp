#include "preview_group.h"

#include "Render/depth_view_pass.h"
#include "Render/linear_depth_view_pass.h"
#include "Render/normal_view_pass.h"
#include "Render/render_graph.h"

void PreviewGroup::Build(RenderGraph& graph, const Inputs& inputs, const Props& props) {
  output_.normal_preview_rt = graph.CreateRenderTexture({
    .name = "normal_preview_rt",
    .format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .width = props.context.width,
    .height = props.context.height,
    .device = props.context.device,
  });

  output_.linear_depth_preview_rt = graph.CreateRenderTexture({
    .name = "linear_depth_preview_rt",
    .format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .width = props.context.width,
    .height = props.context.height,
    .device = props.context.device,
  });

  output_.depth_preview_rt = graph.CreateRenderTexture({
    .name = "depth_preview_rt",
    .format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .width = props.context.width,
    .height = props.context.height,
    .device = props.context.device,
  });

  PassSetup normal_view_setup;
  normal_view_setup.resource_writes = {output_.normal_preview_rt};
  normal_view_setup.resource_reads = {inputs.normal_depth_rt};

  AddPass(graph, std::make_unique<NormalViewPass>(NormalViewPassProps{
    .device = props.context.device,
    .shader_manager = props.context.shader_manager,
    .pass_setup = normal_view_setup,
    .source_handle = inputs.normal_depth_rt,
  }));

  PassSetup linear_depth_view_setup;
  linear_depth_view_setup.resource_writes = {output_.linear_depth_preview_rt};
  linear_depth_view_setup.resource_reads = {inputs.normal_depth_rt};

  AddPass(graph, std::make_unique<LinearDepthViewPass>(LinearDepthViewPassProps{
    .device = props.context.device,
    .shader_manager = props.context.shader_manager,
    .pass_setup = linear_depth_view_setup,
    .source_handle = inputs.normal_depth_rt,
    .config = props.depth_preview_config,
  }));

  PassSetup depth_preview_setup;
  depth_preview_setup.resource_writes = {output_.depth_preview_rt};
  depth_preview_setup.resource_reads = {inputs.scene_depth};

  AddPass(graph, std::make_unique<DepthViewPass>(DepthViewPassProps{
    .device = props.context.device,
    .shader_manager = props.context.shader_manager,
    .pass_setup = depth_preview_setup,
    .depth_handle = inputs.scene_depth,
    .config = props.depth_preview_config,
  }));
}
