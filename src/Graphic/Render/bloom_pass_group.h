#pragma once

#include <cstdint>

#include "Render/pass_group.h"

struct BloomConfig;
class RenderGraph;
class ShaderManager;
struct ID3D12Device;

struct BloomBuildProps {
  ID3D12Device* device;
  ShaderManager* shader_manager;
  const BloomConfig* config;
  uint32_t screen_width;
  uint32_t screen_height;
};

class BloomPassGroup : public PassGroup {
 public:
  explicit BloomPassGroup(const char* group_name) : PassGroup(group_name) {}

  static constexpr uint32_t MAX_MIP_LEVELS = 8u;

  void Build(RenderGraph& graph, RenderGraphHandle scene_hdr, const BloomBuildProps& props);

  RenderGraphHandle GetBloomOutput() const {
    return bloom_output_;
  }

 private:
  RenderGraphHandle bloom_output_ = RenderGraphHandle::Invalid;
};
