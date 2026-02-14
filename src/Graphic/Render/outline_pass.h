#pragma once

#include "Render/pass_group.h"
#include "Render/pipeline_context.h"
#include "Render/render_graph_handle.h"

struct OutlineConfig;
class RenderGraph;

struct OutlineBuildProps {
  const PipelineContext& context;
  const OutlineConfig* config;
};

class OutlinePassGroup : public PassGroup {
 public:
  OutlinePassGroup() : PassGroup("Outline") {}

  void Build(RenderGraph& graph, RenderGraphHandle tonemap_rt, RenderGraphHandle normal_depth_rt, const OutlineBuildProps& props);

  RenderGraphHandle GetOutput() const {
    return outline_rt_;
  }

 private:
  RenderGraphHandle outline_rt_ = RenderGraphHandle::Invalid;
};
