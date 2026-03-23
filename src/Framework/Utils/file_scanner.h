/**
 * @file file_scanner.h
 * @brief Filesystem directory scanning utilities.
 */
#pragma once

#include <algorithm>
#include <filesystem>
#include <initializer_list>
#include <string>
#include <vector>

namespace file_utils {

inline std::vector<std::string> ScanDirectory(const std::filesystem::path& directory, std::initializer_list<const char*> extensions) {
  std::vector<std::string> result;
  if (!std::filesystem::exists(directory)) return result;

  for (const auto& entry : std::filesystem::directory_iterator(directory)) {
    if (!entry.is_regular_file()) continue;
    auto ext = entry.path().extension().string();
    for (const char* allowed : extensions) {
      if (ext == allowed) {
        result.push_back(entry.path().filename().string());
        break;
      }
    }
  }
  std::sort(result.begin(), result.end());
  return result;
}

}  // namespace file_utils
