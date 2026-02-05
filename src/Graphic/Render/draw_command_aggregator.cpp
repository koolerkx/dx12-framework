#include "draw_command_aggregator.h"

#include <unordered_map>

std::vector<DrawCommand> DrawCommandAggregator::Aggregate(const std::vector<DrawCommand>& input) {
  std::vector<DrawCommand> output;
  output.reserve(input.size());

  std::unordered_map<AggregationKey, std::vector<const DrawCommand*>, AggregationKeyHash> groups;

  for (const auto& cmd : input) {
    if (cmd.IsInstanced() || !SupportsInstancing(cmd.material)) {
      output.push_back(cmd);
      continue;
    }

    AggregationKey key{.material = cmd.material, .mesh = cmd.mesh, .sampler_index = cmd.material_instance.sampler_index};
    groups[key].push_back(&cmd);
  }

  for (const auto& [key, commands] : groups) {
    if (commands.size() == 1) {
      output.push_back(*commands[0]);
      continue;
    }

    DrawCommand instanced_cmd;
    instanced_cmd.material = key.material;
    instanced_cmd.mesh = key.mesh;
    instanced_cmd.material_instance = commands[0]->material_instance;
    instanced_cmd.layer = commands[0]->layer;
    instanced_cmd.tags = commands[0]->tags;
    instanced_cmd.instances.reserve(commands.size());

    float min_depth = commands[0]->depth;

    for (const auto* cmd_ptr : commands) {
      instanced_cmd.instances.push_back(ConvertToInstance(*cmd_ptr));
      min_depth = (std::min)(min_depth, cmd_ptr->depth);
    }

    instanced_cmd.depth = min_depth;
    output.push_back(std::move(instanced_cmd));
  }

  return output;
}

bool DrawCommandAggregator::SupportsInstancing(const Material* material) {
  if (!material) {
    return false;
  }
  return material->SupportsInstancing();
}

SpriteInstanceData DrawCommandAggregator::ConvertToInstance(const DrawCommand& cmd) {
  return SpriteInstanceData{.world_matrix = cmd.world_matrix, .color = cmd.color, .uv_offset = cmd.uv_offset, .uv_scale = cmd.uv_scale};
}
