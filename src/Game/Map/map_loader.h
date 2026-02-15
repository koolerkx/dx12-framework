#pragma once
#include <filesystem>
#include <optional>

#include "map_data.h"

class MapLoader {
 public:
  static std::optional<MapData> Load(const std::filesystem::path& path);
};
