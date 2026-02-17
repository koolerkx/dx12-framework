#pragma once

#include "Render/pass_group.h"
#include "Render/pipeline_context.h"
#include "Render/render_graph_handle.h"

struct ChromaticAberrationConfig;
class RenderGraph;

struct ChromaticAberrationBuildProps {
  const PipelineContext& context;
  const ChromaticAberrationConfig* config;
};

class ChromaticAberrationPassGroup : public PassGroup {
 public:
  ChromaticAberrationPassGroup() : PassGroup("ChromaticAberration") {}

  void Build(RenderGraph& graph, RenderGraphHandle input_rt, const ChromaticAberrationBuildProps& props);

  RenderGraphHandle GetOutput() const {
    return chromatic_aberration_rt_;
  }

 private:
  RenderGraphHandle chromatic_aberration_rt_ = RenderGraphHandle::Invalid;
};
