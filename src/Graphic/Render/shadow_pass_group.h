#pragma once

#include <cstdint>
#include <vector>

#include "Render/pass_group.h"
#include "Render/pipeline_context.h"
#include "Render/render_graph_handle.h"

class RenderGraph;
struct ShadowFrameData;

class ShadowPassGroup : public PassGroup {
 public:
  ShadowPassGroup() : PassGroup("Shadow") {}

  struct Props {
    const PipelineContext& context;
    uint32_t cascade_count;
    uint32_t resolution;
    ShadowFrameData* shadow_data;
    RenderGraphHandle* shadow_depth_handles;
  };

  void Build(RenderGraph& graph, const Props& props);

  const std::vector<RenderGraphHandle>& GetShadowMaps() const {
    return shadow_maps_;
  }

 private:
  std::vector<RenderGraphHandle> shadow_maps_;
};
