/**
@filename logger.h
@brief Public logging API and data structures. Defines log levels, categories, log entry format, and the static Logger interface used
throughout the engine.
@author Kooler Fan
**/
#pragma once

#include <chrono>
#include <format>
#include <memory>
#include <source_location>
#include <string>
#include <string_view>
#include <vector>

#include "Framework/Logging/logger_config.h"
#include "Framework/Logging/sinks.h"


enum class LogLevel : uint8_t { Trace, Debug, Info, Warn, Error, Fatal };

enum class LogCategory : uint8_t {
  Core,
  Graphic,
  Resource,
  Game,
  UI,
  Editor,
  Validation,
};

struct LogEntry {
  LogLevel level = LogLevel::Info;
  LogCategory category = LogCategory::Core;
  std::chrono::system_clock::time_point timestamp{};
  std::source_location loc{};
  std::string message;
};

class Logger final {
 public:
  Logger() = delete;

  static void Init(LoggerConfig cfg, std::vector<std::unique_ptr<ILogSink>> sinks);
  static void Init(std::vector<std::unique_ptr<ILogSink>> sinks);
  static void Shutdown();

  static void Flush();
  static void EnterPanic() noexcept;

  [[nodiscard]] static bool IsInitialized();
  [[nodiscard]] static bool IsEnabled(LogLevel level, LogCategory category);
  [[nodiscard]] static LoggerConfig GetConfig();

  static void Log(
    LogLevel level, LogCategory category, std::string message, const std::source_location& loc = std::source_location::current());

  static void EmitDirectMinimal(LogLevel level,
    LogCategory category,
    std::string_view message,
    const std::source_location& loc = std::source_location::current()) noexcept;

  [[nodiscard]] static constexpr std::source_location Here(const std::source_location& loc = std::source_location::current()) noexcept {
    return loc;
  }

  // Log with format, using C++20 std::format
  template <class... Args>
  static void Logf(LogLevel level, LogCategory category, const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args) {
    if (!IsEnabled(level, category)) {
      return;
    }
    Log(level, category, std::format(fmt, std::forward<Args>(args)...), loc);
  }

  static void Logv(LogLevel level,
    LogCategory category,
    std::string_view fmt,
    std::format_args args,
    const std::source_location& loc = std::source_location::current());

 private:
  struct State;

  static State& GetState();

  static void Emit(const LogEntry& entry);
};
