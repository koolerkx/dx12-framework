#pragma once

#include "Render/pass_group.h"
#include "Render/pipeline_context.h"
#include "Render/render_graph_handle.h"

class RenderGraph;

class PrepassGroup : public PassGroup {
 public:
  PrepassGroup() : PassGroup("Prepass") {}

  struct Props {
    const PipelineContext& context;
  };

  void Build(RenderGraph& graph, const Props& props);

  RenderGraphHandle GetNormalDepthRT() const {
    return normal_depth_rt_;
  }

  RenderGraphHandle GetPrepassDepth() const {
    return prepass_depth_;
  }

 private:
  RenderGraphHandle normal_depth_rt_ = RenderGraphHandle::Invalid;
  RenderGraphHandle prepass_depth_ = RenderGraphHandle::Invalid;
};
