#pragma once

#include <d3d12.h>
#include <dxgiformat.h>
#include <windows.h>

#include <stdexcept>
#include <string>
#include <string_view>

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
 * @brief Convert UTF-8 string to UTF-16 wstring without throwing exceptions.
 * @param utf8_str std::string
 * @return std::wstring (empty on failure)
 */
inline std::wstring Utf8ToWstringNoThrow(std::string_view utf8_str) noexcept {
  if (utf8_str.empty()) return std::wstring();

  int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8_str.data(), (int)utf8_str.size(), NULL, 0);
  if (size_needed == 0) {
    return std::wstring();
  }

  std::wstring wstr(size_needed, 0);
  MultiByteToWideChar(CP_UTF8, 0, utf8_str.data(), (int)utf8_str.size(), &wstr[0], size_needed);
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

inline const std::string GetDxgiFormatName(DXGI_FORMAT format) {
  switch (format) {
    case DXGI_FORMAT_R8G8B8A8_UNORM:
      return "R8G8B8A8_UNORM";  // 28
    case DXGI_FORMAT_R16G16B16A16_UNORM:
      return "R16G16B16A16_UNORM";  // 11
    case DXGI_FORMAT_R16_UNORM:
      return "R16_UNORM";  // 56
    case DXGI_FORMAT_R8_UNORM:
      return "R8_UNORM";  // 61
    case DXGI_FORMAT_B8G8R8A8_UNORM:
      return "B8G8R8A8_UNORM";  // 87
    default:
      return "UNKNOWN";
  }
}

// Debug Wrapper
#if defined(DEBUG) || defined(_DEBUG)
struct DebugCommandGroupWrapper final {
  ID3D12GraphicsCommandList* command_list_{};

  DebugCommandGroupWrapper(ID3D12GraphicsCommandList* command_list, const wchar_t* label) noexcept : command_list_(command_list) {
    if (!command_list_ || !label) return;
    const UINT bytes = static_cast<UINT>((wcslen(label) + 1) * sizeof(wchar_t));
    command_list_->BeginEvent(0, label, bytes);
  }

  ~DebugCommandGroupWrapper() noexcept {
    if (command_list_) command_list_->EndEvent();
  }

  DebugCommandGroupWrapper(const DebugCommandGroupWrapper&) = delete;
  DebugCommandGroupWrapper& operator=(const DebugCommandGroupWrapper&) = delete;
  DebugCommandGroupWrapper(DebugCommandGroupWrapper&&) = delete;
  DebugCommandGroupWrapper& operator=(DebugCommandGroupWrapper&&) = delete;
};

template <class Fn>
inline void CommandListEventGroup(ID3D12GraphicsCommandList* command_list, const wchar_t* label, Fn&& fn) {
  DebugCommandGroupWrapper wrapper(command_list, label);
  std::forward<Fn>(fn)();
}
#else
template <class Fn>
inline void CommandListEventGroup(ID3D12GraphicsCommandList* /*command_list*/, const wchar_t* /*label*/, Fn&& fn) {
  std::forward<Fn>(fn)();
}
#endif

constexpr uint32_t HashString(std::string_view str) {
  uint32_t hash = 2166136261u;
  for (char c : str) {
    hash ^= static_cast<uint32_t>(c);
    hash *= 16777619u;
  }
  return hash;
}

}  // namespace utils
