#include "map_data.h"

XZBounds ComputeGroundBounds(const MapData& map_data) {
  XZBounds bounds;
  for (const auto& layer : map_data.layers) {
    if (layer.id != "ground") continue;
    for (const auto& item : layer.items) {
      float world_x = item.transform.x + map_data.origin_x;
      float world_z = item.transform.z + map_data.origin_z;
      float half_sx = 0.5f * item.transform.scale_x;
      float half_sz = 0.5f * item.transform.scale_z;
      bounds.Expand(world_x, world_z, half_sx, half_sz);
    }
  }
  return bounds;
}
