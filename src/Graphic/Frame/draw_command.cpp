#include "draw_command.h"

#include <bit>

// Key: [RS hash 16-bit | PSO hash 16-bit | Depth 32-bit]
uint64_t DrawCommand::GetSortKeyForOpaque(bool front_to_back) const {
  if (!material) {
    return UINT64_MAX;
  }

  uint32_t rs_key = material->GetRootSignatureKey() & 0xFFFF;
  uint32_t pso_key = material->GetPSOKey() & 0xFFFF;
  uint64_t compressed_material = (static_cast<uint64_t>(rs_key) << 48) | (static_cast<uint64_t>(pso_key) << 32);

  uint32_t depth_key = std::bit_cast<uint32_t>(depth);
  if (!front_to_back) {
    depth_key = ~depth_key;
  }

  return compressed_material | depth_key;
}

// Key: [Depth 32-bit | RS hash 16-bit | PSO hash 16-bit]
uint64_t DrawCommand::GetSortKeyForTransparent(bool front_to_back) const {
  if (!material) {
    return UINT64_MAX;
  }

  uint32_t depth_key = std::bit_cast<uint32_t>(depth);
  if (!front_to_back) {
    depth_key = ~depth_key;
  }

  uint32_t rs_key = material->GetRootSignatureKey() & 0xFFFF;
  uint32_t pso_key = material->GetPSOKey() & 0xFFFF;
  uint64_t material_key = (static_cast<uint64_t>(rs_key) << 16) | static_cast<uint64_t>(pso_key);

  return (static_cast<uint64_t>(depth_key) << 32) | material_key;
}

uint64_t DrawCommand::GetSortKey(bool front_to_back, bool depth_first) const {
  return depth_first ? GetSortKeyForTransparent(front_to_back) : GetSortKeyForOpaque(front_to_back);
}

bool RenderPassFilter::Matches(const RenderCommand& cmd) const {
  // Check layer match
  if (cmd.layer != target_layer) {
    return false;
  }

  // Check required tags (must have all)
  if (required_tags != 0 && !HasAllTags(cmd.tags, required_tags)) {
    return false;
  }

  // Check excluded tags (must not have any)
  if (excluded_tags != 0 && (cmd.tags & excluded_tags) != 0) {
    return false;
  }

  return true;
}
