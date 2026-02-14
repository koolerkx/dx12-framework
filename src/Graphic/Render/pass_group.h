#pragma once

#include <memory>

#include "Render/render_pass.h"

class RenderGraph;

class PassGroup {
 public:
  explicit PassGroup(const char* group_name) : group_name_(group_name) {}

 protected:
  void AddPass(RenderGraph& graph, std::unique_ptr<IRenderPass> pass);

 private:
  const char* group_name_;
};
