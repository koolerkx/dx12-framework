/**
 * @file console_sink.cpp
 * @brief Logging Sink implementation that outputs log messages to a Windows console. Handles attaching to an existing parent console or
 * safely ignoring output when no console is available.
 * @author Kooler Fan
 **/

#include "Framework/Logging/sinks.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <string>

#include "Framework/Core/utils.h"
#include "Framework/Logging/logger.h"

using utils::Utf8ToWstringNoThrow;

ConsoleSink::ConsoleSink() {
  AttachToParentConsoleIfPossible();
}

bool ConsoleSink::IsAttached() const {
  return attached_ && output_handle_ != nullptr;
}

void ConsoleSink::AttachToParentConsoleIfPossible() {
  attached_ = false;
  output_handle_ = nullptr;

  if (AttachConsole(ATTACH_PARENT_PROCESS) == 0) {
    return;
  }

  HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
  if (handle == nullptr || handle == INVALID_HANDLE_VALUE) {
    return;
  }

  output_handle_ = handle;
  attached_ = true;
}

void ConsoleSink::OnLog(const LogEntry& entry) {
  if (!IsAttached()) {
    return;
  }

  std::wstring msg = Utf8ToWstringNoThrow(entry.message);
  msg.append(L"\n");

  DWORD written = 0;
  WriteConsoleW(static_cast<HANDLE>(output_handle_), msg.data(), static_cast<DWORD>(msg.size()), &written, nullptr);
}

void ConsoleSink::Flush() {
  if (!IsAttached()) {
    return;
  }
  FlushFileBuffers(static_cast<HANDLE>(output_handle_));
}
