#pragma once

#include <filesystem>
#include <optional>
#include <string>

enum class FilePathMode { WorkingDir, ExeDir /*, LocalAppData_future*/ };

enum class OverflowPolicy { DropLowFirst /*, Block_future, DropOldest_future, DropNewest_future*/ };

struct LoggerConfig {
  size_t queue_capacity = 8192;
  OverflowPolicy overflow_policy = OverflowPolicy::DropLowFirst;

  FilePathMode file_path_mode = FilePathMode::ExeDir;
  std::optional<std::filesystem::path> file_dir_override;

  std::string app_name_fallback = "dx12-game-with-framework";
};







