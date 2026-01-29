/**
@filename path_utils.h
@brief Path utilities for resolving application directories and project root.
@author Kooler Fan
**/
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <filesystem>
#include <optional>
#include <string>

inline std::optional<std::filesystem::path> GetExeDir() {
  wchar_t buffer[MAX_PATH]{};
  const DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
  if (len == 0 || len == MAX_PATH) {
    return std::nullopt;
  }

  std::filesystem::path exe_path(buffer);
  return exe_path.parent_path();
}

inline std::optional<std::filesystem::path> FindProjectRoot() {
  auto current = std::filesystem::current_path();

  // Search for CMakeLists.txt or .git directory
  while (true) {
    if (std::filesystem::exists(current / "CMakeLists.txt") || std::filesystem::exists(current / ".git")) {
      return current;
    }

    auto parent = current.parent_path();
    if (parent == current) {
      return std::nullopt;
    }
    current = parent;
  }
}

inline std::string GetAppNameUtf8(std::string_view fallback = "app") {
  wchar_t buffer[MAX_PATH]{};
  const DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
  if (len == 0 || len == MAX_PATH) {
    return std::string(fallback);
  }

  std::filesystem::path exe_path(buffer);
  std::string filename = exe_path.stem().string();
  return filename.empty() ? std::string(fallback) : filename;
}
