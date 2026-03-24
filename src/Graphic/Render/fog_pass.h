#pragma once

#include "Render/pass_group.h"

struct FogConfig;
class RenderGraph;

struct FogBuildProps {
  const PipelineContext& context;
  const FogConfig* config;
};

class FogPassGroup : public PassGroup {
 public:
  FogPassGroup() : PassGroup("Fog") {}

  void Build(RenderGraph& graph, RenderGraphHandle input_rt, RenderGraphHandle normal_depth_rt, const FogBuildProps& props);

  RenderGraphHandle GetOutput() const {
    return fog_rt_;
  }

 private:
  RenderGraphHandle fog_rt_ = RenderGraphHandle::Invalid;
};
