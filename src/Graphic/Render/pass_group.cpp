#include "pass_group.h"

#include "Render/render_graph.h"

void PassGroup::AddPass(RenderGraph& graph, std::unique_ptr<IRenderPass> pass) {
  pass->SetGroupName(group_name_);
  graph.AddPass(std::move(pass));
}
