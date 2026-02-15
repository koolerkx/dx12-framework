#pragma once
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
