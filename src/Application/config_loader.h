#pragma once
#include <filesystem>

#include "app_config.h"

class ConfigLoader {
 public:
  static AppConfig LoadFromFile(const std::filesystem::path& path);
};
