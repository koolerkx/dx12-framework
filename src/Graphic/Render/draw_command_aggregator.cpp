#include "draw_command_aggregator.h"

#include <unordered_map>

std::vector<RenderCommand> DrawCommandAggregator::Aggregate(const std::vector<RenderCommand>& input) {
  std::vector<RenderCommand> output;
  output.reserve(input.size());

  // Group commands by aggregation key
  std::unordered_map<AggregationKey, std::vector<const RenderCommand*>, AggregationKeyHash> groups;

  for (const auto& render_cmd : input) {
    const DrawCommand& cmd = render_cmd.command;

    // Pass through if already instanced or doesn't support instancing
    if (cmd.IsInstanced() || !SupportsInstancing(cmd.material)) {
      output.push_back(render_cmd);
      continue;
    }

    // Group by material + mesh + sampler
    AggregationKey key{.material = cmd.material, .mesh = cmd.mesh, .sampler_index = cmd.material_instance.sampler_index};

    groups[key].push_back(&render_cmd);
  }

  // Convert groups to instanced commands
  for (const auto& [key, commands] : groups) {
    if (commands.size() == 1) {
      // Single command, no need to aggregate
      output.push_back(*commands[0]);
      continue;
    }

    // Create instanced command
    RenderCommand instanced_cmd;
    instanced_cmd.layer = commands[0]->layer;
    instanced_cmd.tags = commands[0]->tags;

    DrawCommand& draw = instanced_cmd.command;
    draw.material = key.material;
    draw.mesh = key.mesh;
    draw.material_instance = commands[0]->command.material_instance;
    draw.instances.reserve(commands.size());

    // Aggregate depth (use minimum for front-to-back sorting)
    float min_depth = commands[0]->command.depth;

    for (const auto* cmd_ptr : commands) {
      const DrawCommand& cmd = cmd_ptr->command;
      draw.instances.push_back(ConvertToInstance(cmd));
      min_depth = (std::min)(min_depth, cmd.depth);
    }

    draw.depth = min_depth;

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
