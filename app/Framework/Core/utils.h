#pragma once

#include <windows.h>

#include <stdexcept>
#include <string>

namespace utils {

/**
 * @brief Convert UTF-8 string to UTF-16 wstring.
 * @param utf8_str std::string
 * @return std::wstring
 * @throws std::runtime_error On conversion failure.
 */
inline std::wstring utf8_to_wstring(const std::string& utf8_str) {
  if (utf8_str.empty()) return std::wstring();

  int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), (int)utf8_str.size(), NULL, 0);
  if (size_needed == 0) {
    throw std::runtime_error("Utils: UTF-8 to UTF-16 conversion failed");
  }

  std::wstring wstr(size_needed, 0);
  MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), (int)utf8_str.size(), &wstr[0], size_needed);
  return wstr;
}

/**
 * @brief Convert UTF-8 string to UTF-16 wstring.
 * @param wstr std::wstring
 * @return std::string
 * @throws std::runtime_error On conversion failure.
 */
inline std::string wstring_to_utf8(const std::wstring& wstr) {
  if (wstr.empty()) return std::string();

  int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
  if (size_needed == 0) {
    throw std::runtime_error("Utils: UTF-16 to UTF-8 conversion failed");
  }

  std::string utf8_str(size_needed, 0);
  WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &utf8_str[0], size_needed, NULL, NULL);
  return utf8_str;
}
}  // namespace utils
