#pragma once
#include <algorithm>
#include <limits>
#include <string>
#include <vector>

struct MapMeshResource {
  std::string id;
  std::string path;
};

struct MapItemTransform {
  float x = 0.0f;
  float z = 0.0f;
  float rotation_deg = 0.0f;
  float scale_x = 1.0f;
  float scale_z = 1.0f;
};

struct MapItem {
  std::string name;
  std::string mesh_id;
  MapItemTransform transform;
};

struct MapLayer {
  std::string id;
  float y_offset = 0.0f;
  std::vector<MapItem> items;
};

struct MapData {
  std::string name;
  float origin_x = 0.0f;
  float origin_z = 0.0f;
  std::vector<MapMeshResource> mesh_resources;
  std::vector<MapLayer> layers;
};

struct XZBounds {
  float min_x = (std::numeric_limits<float>::max)();
  float max_x = (std::numeric_limits<float>::lowest)();
  float min_z = (std::numeric_limits<float>::max)();
  float max_z = (std::numeric_limits<float>::lowest)();

  void Expand(float x, float z, float half_sx, float half_sz) {
    min_x = (std::min)(min_x, x - half_sx);
    max_x = (std::max)(max_x, x + half_sx);
    min_z = (std::min)(min_z, z - half_sz);
    max_z = (std::max)(max_z, z + half_sz);
  }
};

XZBounds ComputeGroundBounds(const MapData& map_data);
