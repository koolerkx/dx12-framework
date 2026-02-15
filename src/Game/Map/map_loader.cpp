#include "map_loader.h"

#include "Framework/Logging/logger.h"
#include "Framework/Serialize/serialize_document.h"

std::optional<MapData> MapLoader::Load(const std::filesystem::path& path) {
  framework::SerializeDocument doc;
  if (!doc.LoadFromFile(path)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Game, Logger::Here(), "[MapLoader] Failed to load: {}", path.string());
    return std::nullopt;
  }

  auto& root = doc.Root();
  MapData data;
  data.name = root.ReadString("name");

  float ox = 0.0f, oz = 0.0f;
  root.ReadVec2("origin", ox, oz);
  data.origin_x = ox;
  data.origin_z = oz;

  auto resources_node = root.GetMap("resources");
  size_t mesh_count = resources_node.GetSequenceSize("meshes");
  data.mesh_resources.reserve(mesh_count);
  for (size_t i = 0; i < mesh_count; ++i) {
    auto mesh_node = resources_node.GetSequence("meshes").GetSequenceElement(i);
    MapMeshResource res;
    res.id = mesh_node.ReadString("id");
    res.path = mesh_node.ReadString("path");
    data.mesh_resources.push_back(std::move(res));
  }

  size_t layer_count = root.GetSequenceSize("layers");
  data.layers.reserve(layer_count);
  for (size_t i = 0; i < layer_count; ++i) {
    auto layer_node = root.GetSequence("layers").GetSequenceElement(i);
    MapLayer layer;
    layer.id = layer_node.ReadString("id");
    layer.y_offset = layer_node.ReadFloat("y_offset");

    size_t item_count = layer_node.GetSequenceSize("items");
    layer.items.reserve(item_count);
    for (size_t j = 0; j < item_count; ++j) {
      auto item_node = layer_node.GetSequence("items").GetSequenceElement(j);
      MapItem item;
      item.name = item_node.ReadString("name");
      item.mesh_id = item_node.ReadString("mesh");

      auto transform_node = item_node.GetMap("transform");
      float px = 0.0f, pz = 0.0f;
      transform_node.ReadVec2("position", px, pz);
      item.transform.x = px;
      item.transform.z = pz;
      item.transform.rotation_deg = transform_node.ReadFloat("rotation");

      float sx = 1.0f, sz = 1.0f;
      transform_node.ReadVec2("scale", sx, sz);
      item.transform.scale_x = sx;
      item.transform.scale_z = sz;

      layer.items.push_back(std::move(item));
    }

    data.layers.push_back(std::move(layer));
  }

  Logger::LogFormat(LogLevel::Info,
    LogCategory::Game,
    Logger::Here(),
    "[MapLoader] Loaded '{}': {} meshes, {} layers",
    data.name,
    data.mesh_resources.size(),
    data.layers.size());

  return data;
}
