#include "post_process_group.h"

#include "Render/bloom_pass_group.h"
#include "Render/render_graph.h"
#include "Render/tone_map_pass.h"
#include "Rendering/hdr_config.h"

void PostProcessGroup::Build(RenderGraph& graph, RenderGraphHandle scene_rt, const Props& props) {
  tonemap_rt_ = graph.CreateRenderTexture({
    .name = "tonemap_rt",
    .format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .width = props.context.width,
    .height = props.context.height,
    .device = props.context.device,
  });

  RenderGraphHandle bloom_output = RenderGraphHandle::Invalid;
  if (props.bloom_config->enabled) {
    BloomPassGroup bloom_group("Post Process: Bloom");
    bloom_group.Build(graph, scene_rt, {
      .device = props.context.device,
      .shader_manager = props.context.shader_manager,
      .config = props.bloom_config,
      .screen_width = props.context.width,
      .screen_height = props.context.height,
    });
    bloom_output = bloom_group.GetBloomOutput();
  }

  PassSetup tonemap_setup;
  tonemap_setup.resource_writes = {tonemap_rt_};
  tonemap_setup.resource_reads = {scene_rt};
  if (bloom_output != RenderGraphHandle::Invalid) {
    tonemap_setup.resource_reads.push_back(bloom_output);
  }

  AddPass(graph, std::make_unique<ToneMapPass>(ToneMapPassProps{
    .device = props.context.device,
    .material_manager = props.material_manager,
    .shader_manager = props.context.shader_manager,
    .pass_setup = tonemap_setup,
    .hdr_handle = scene_rt,
    .debug = props.hdr_debug,
    .bloom_handle = bloom_output,
    .bloom_config = props.bloom_config,
  }));
}
