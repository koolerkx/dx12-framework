#pragma once

#include "Render/pass_group.h"
#include "Render/pipeline_context.h"
#include "Render/render_graph_handle.h"

struct SMAAConfig;
class RenderGraph;
class TextureManager;

struct SMAAPipelineBuildProps {
  const PipelineContext& context;
  TextureManager* texture_manager;
  const SMAAConfig* config;
};

class SMAAPassGroup : public PassGroup {
 public:
  SMAAPassGroup() : PassGroup("SMAA") {}

  void Build(RenderGraph& graph, RenderGraphHandle ldr_input, const SMAAPipelineBuildProps& props);

  RenderGraphHandle GetOutput() const {
    return smaa_output_;
  }

 private:
  RenderGraphHandle smaa_output_ = RenderGraphHandle::Invalid;
};
