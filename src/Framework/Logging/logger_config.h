#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <chrono>

struct LoggerConfig {
  // size of log queue, the log exceed this cound will be handle by overflow policy
  size_t queue_capacity = 8192;

  // size of batch
  size_t batch_size = 2048;

  // logging delay
  std::chrono::milliseconds worker_wait_ms = std::chrono::milliseconds(25);

  // policy when queu is full
  enum class OverflowPolicy {
    DropLowFirst  // drop low priority message
    /*, Block_future, DropOldest_future, DropNewest_future*/
  };
  OverflowPolicy overflow_policy = OverflowPolicy::DropLowFirst;

  enum class FilePathMode {
    WorkingDir,  // based on CMakeLists.txt dir
    ExeDir       // based on exe dir
    /*, LocalAppData_future*/
  };
  FilePathMode file_path_mode = FilePathMode::ExeDir;
  std::optional<std::filesystem::path> file_dir_override;

  // log file name fallback
  std::string app_name_fallback = "dx12-framework";
};
