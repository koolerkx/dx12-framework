#pragma once

#include "Render/pass_group.h"

class RenderGraph;
class MaterialManager;
struct BloomConfig;
struct HdrDebug;

class PostProcessGroup : public PassGroup {
 public:
  PostProcessGroup() : PassGroup("Post Process") {}

  struct Props {
    const PipelineContext& context;
    MaterialManager* material_manager;
    const BloomConfig* bloom_config;
    HdrDebug* hdr_debug;
  };

  void Build(RenderGraph& graph, RenderGraphHandle scene_rt, const Props& props);

  RenderGraphHandle GetLdrOutput() const {
    return tonemap_rt_;
  }

 private:
  RenderGraphHandle tonemap_rt_ = RenderGraphHandle::Invalid;
};
