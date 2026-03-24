#pragma once

#include "Render/pass_group.h"

struct VignetteConfig;
class RenderGraph;

struct VignetteBuildProps {
  const PipelineContext& context;
  const VignetteConfig* config;
};

class VignettePassGroup : public PassGroup {
 public:
  VignettePassGroup() : PassGroup("Vignette") {}

  void Build(RenderGraph& graph, RenderGraphHandle input_rt, const VignetteBuildProps& props);

  RenderGraphHandle GetOutput() const {
    return vignette_rt_;
  }

 private:
  RenderGraphHandle vignette_rt_ = RenderGraphHandle::Invalid;
};
