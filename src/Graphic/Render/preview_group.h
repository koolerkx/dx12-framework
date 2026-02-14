#pragma once

#include "Render/pass_group.h"
#include "Render/pipeline_context.h"
#include "Render/render_graph_handle.h"

class RenderGraph;
struct DepthViewConfig;

class PreviewGroup : public PassGroup {
 public:
  PreviewGroup() : PassGroup("Preview") {}

  struct Inputs {
    RenderGraphHandle normal_depth_rt;
    RenderGraphHandle scene_depth;
  };

  struct Props {
    const PipelineContext& context;
    const DepthViewConfig* depth_preview_config;
  };

  struct Output {
    RenderGraphHandle normal_preview_rt = RenderGraphHandle::Invalid;
    RenderGraphHandle linear_depth_preview_rt = RenderGraphHandle::Invalid;
    RenderGraphHandle depth_preview_rt = RenderGraphHandle::Invalid;
  };

  void Build(RenderGraph& graph, const Inputs& inputs, const Props& props);

  const Output& GetOutput() const {
    return output_;
  }

 private:
  Output output_;
};
