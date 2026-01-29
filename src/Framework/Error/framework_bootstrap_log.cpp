/**
@filename framework_bootstrap_log.cpp
@brief Minimal, dependency-free logging facility used during early startup or when the main Logger system is unavailable. Writes directly to
debugger output.
@author Kooler Fan
**/
#include "Framework/Error/framework_bootstrap_log.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <array>
#include <cstdint>
#include <string_view>

namespace {

constexpr size_t kMaxWideMessageChars = 1024;

size_t ClampToWideCapacity(int required_chars) {
  if (required_chars <= 0) {
    return 0;
  }
  const size_t as_size = static_cast<size_t>(required_chars);
  return (as_size < kMaxWideMessageChars) ? as_size : (kMaxWideMessageChars - 1);
}

size_t Utf8ToWideTruncated(std::string_view utf8, wchar_t* out, size_t out_capacity) {
  if (out_capacity == 0) {
    return 0;
  }
  out[0] = L'\0';

  if (utf8.empty()) {
    return 0;
  }

  const int required = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), nullptr, 0);
  const size_t to_write =
    (required > 0) ? ((static_cast<size_t>(required) < out_capacity) ? static_cast<size_t>(required) : (out_capacity - 1)) : 0;
  if (to_write == 0) {
    return 0;
  }

  MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), out, static_cast<int>(to_write));
  out[to_write] = L'\0';
  return to_write;
}

}  // namespace

void FrameworkBootstrapLog(std::string_view message, const std::source_location& loc) noexcept {
  std::array<wchar_t, kMaxWideMessageChars> buffer{};

  const uint_least32_t line = loc.line();
  const int prefix_chars = _snwprintf_s(buffer.data(), buffer.size(), _TRUNCATE, L"[bootstrap] line=%u: ", static_cast<unsigned>(line));
  const size_t prefix_len = ClampToWideCapacity(prefix_chars);

  if (prefix_len + 1 < buffer.size()) {
    Utf8ToWideTruncated(message, buffer.data() + prefix_len, buffer.size() - prefix_len);
  }

  OutputDebugStringW(buffer.data());
  OutputDebugStringW(L"\n");
}
