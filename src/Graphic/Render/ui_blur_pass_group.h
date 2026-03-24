#pragma once

#include <cstdint>

#include "Render/pass_group.h"

class RenderGraph;
class ShaderManager;
struct ID3D12Device;

struct UIBlurBuildProps {
  ID3D12Device* device;
  ShaderManager* shader_manager;
  uint32_t screen_width;
  uint32_t screen_height;
};

class UIBlurPassGroup : public PassGroup {
 public:
  explicit UIBlurPassGroup(const char* group_name) : PassGroup(group_name) {}

  static constexpr uint32_t MIP_COUNT = 3u;

  void Build(RenderGraph& graph, RenderGraphHandle scene_source, const UIBlurBuildProps& props);

  RenderGraphHandle GetBlurredOutput() const {
    return blurred_output_;
  }

 private:
  RenderGraphHandle blurred_output_ = RenderGraphHandle::Invalid;
};
