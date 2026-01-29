/**
@filename sinks.h
@brief Defines the ILogSink interface and built-in sink implementations (DebugSink, ConsoleSink) for log output destinations.
@author Kooler Fan
**/
#pragma once

#include <atomic>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <string_view>

#include "Framework/Logging/logger_config.h"

struct LogEntry;

struct ILogSink {
  virtual ~ILogSink() = default;
  virtual void OnLog(const LogEntry& entry) = 0;
  virtual void Flush() {
  }
};

class DebugSink final : public ILogSink {
 public:
  void OnLog(const LogEntry& entry) override;
  void Flush() override;
};

class ConsoleSink final : public ILogSink {
 public:
  ConsoleSink();
  void OnLog(const LogEntry& entry) override;
  void Flush() override;

  [[nodiscard]] bool IsAttached() const;

 private:
  void AttachToParentConsoleIfPossible();

  void* output_handle_ = nullptr;
  bool attached_ = false;
};

class FileSink final : public ILogSink {
 public:
  explicit FileSink(const LoggerConfig& cfg);

  void OnLog(const LogEntry& entry) override;
  void Flush() override;

  [[nodiscard]] bool IsEnabled() const;

 private:
  [[nodiscard]] bool EnsureOpened();
  void WarnOnce(std::string_view message) noexcept;

  LoggerConfig cfg_;

  std::filesystem::path log_dir_;
  std::filesystem::path log_file_;

  std::unique_ptr<std::ofstream> stream_;
  bool enabled_ = false;
  std::atomic<bool> warned_{false};
  std::mutex open_mutex_;
};
