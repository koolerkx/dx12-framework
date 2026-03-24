#pragma once

#include <cstdint>
#include <memory>

#include "Render/render_pass.h"

struct ID3D12Device;
class ShaderManager;
class RenderGraph;

struct PipelineContext {
  ID3D12Device* device;
  ShaderManager* shader_manager;
  uint32_t width;
  uint32_t height;
};

class PassGroup {
 public:
  explicit PassGroup(const char* group_name) : group_name_(group_name) {}

 protected:
  void AddPass(RenderGraph& graph, std::unique_ptr<IRenderPass> pass);

 private:
  const char* group_name_;
};
