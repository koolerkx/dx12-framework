#pragma once

#include "Render/pass_group.h"

struct SSAOConfig;
class RenderGraph;

struct SSAOBuildProps {
  const PipelineContext& context;
  const SSAOConfig* config;
};

class SSAOPassGroup : public PassGroup {
 public:
  SSAOPassGroup() : PassGroup("SSAO") {}

  void Build(RenderGraph& graph, RenderGraphHandle normal_depth_rt, const SSAOBuildProps& props);

  RenderGraphHandle GetAOTexture() const {
    return blurred_ao_;
  }

 private:
  RenderGraphHandle blurred_ao_ = RenderGraphHandle::Invalid;
};
