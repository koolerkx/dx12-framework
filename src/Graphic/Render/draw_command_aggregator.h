#pragma once
#include <vector>

#include "Frame/draw_command.h"

class DrawCommandAggregator {
 public:
  static std::vector<DrawCommand> Aggregate(const std::vector<DrawCommand>& input);

 private:
  struct AggregationKey {
    const Material* material;
    const Mesh* mesh;
    uint32_t sampler_index;

    bool operator==(const AggregationKey& other) const {
      return material == other.material && mesh == other.mesh && sampler_index == other.sampler_index;
    }
  };

  struct AggregationKeyHash {
    size_t operator()(const AggregationKey& key) const {
      size_t h1 = std::hash<const void*>{}(key.material);
      size_t h2 = std::hash<const void*>{}(key.mesh);
      size_t h3 = std::hash<uint32_t>{}(key.sampler_index);
      return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
  };

  static bool SupportsInstancing(const Material* material);
  static SpriteInstanceData ConvertToInstance(const DrawCommand& cmd);
};
