#include <format>
#include <system_error>

#include "Framework/Logging/sinks.h"
#include "Logging/logger_config.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <mutex>

#include "Framework/Error/framework_bootstrap_log.h"
#include "Framework/Logging/logger.h"
#include "Framework/Utils/path_utils.h"

namespace {

using FilePathMode = LoggerConfig::FilePathMode;

std::string_view ToString(FilePathMode mode) {
  switch (mode) {
    case FilePathMode::WorkingDir:
      return "WorkingDir";
    case FilePathMode::ExeDir:
      return "ExeDir";
  }
  return "WorkingDir";
}

std::filesystem::path ResolveLogDirectory(const LoggerConfig& cfg) {
  if (cfg.file_dir_override.has_value()) {
    return *cfg.file_dir_override;
  }

  if (cfg.file_path_mode == FilePathMode::WorkingDir) {
    const auto project_root = FindProjectRoot();
    if (project_root.has_value()) {
      return *project_root / "logs";
    }
    return std::filesystem::current_path() / "logs";
  }

  const auto exe_dir = GetExeDir();
  if (!exe_dir.has_value()) {
    return {};
  }
  return *exe_dir / "logs";
}

std::string MakeTimestampLocalCompact() {
  SYSTEMTIME st{};
  GetLocalTime(&st);
  return std::format("{:04}{:02}{:02}_{:02}{:02}{:02}",
    static_cast<int>(st.wYear),
    static_cast<int>(st.wMonth),
    static_cast<int>(st.wDay),
    static_cast<int>(st.wHour),
    static_cast<int>(st.wMinute),
    static_cast<int>(st.wSecond));
}

std::filesystem::path MakeLogFilePath(const std::filesystem::path& dir, const LoggerConfig& cfg) {
  const std::string app = GetAppNameUtf8(cfg.app_name_fallback);
  const std::string ts = MakeTimestampLocalCompact();
  const DWORD pid = GetCurrentProcessId();

  const std::string base = std::format("{}_{}_{}", app, ts, pid);
  return dir / (base + ".log");
}

}  // namespace

FileSink::FileSink(const LoggerConfig& cfg) : cfg_(cfg) {
}

bool FileSink::IsEnabled() const {
  return enabled_;
}

void FileSink::WarnOnce(std::string_view message) noexcept {
  if (warned_.exchange(true)) {
    return;
  }
  FrameworkBootstrapLog(message);
}

[[nodiscard]] bool FileSink::EnsureOpened() {
  {
    std::lock_guard<std::mutex> lock(open_mutex_);
    if (enabled_ && stream_) {
      return true;
    }
  }

  std::lock_guard<std::mutex> lock(open_mutex_);

  if (enabled_ && stream_) {
    return true;
  }

  log_dir_ = ResolveLogDirectory(cfg_);
  if (log_dir_.empty()) {
    const std::string msg = std::format("[logger] FileSink disabled: cannot resolve log dir (mode={})", ToString(cfg_.file_path_mode));
    WarnOnce(msg);
    enabled_ = false;
    return false;
  }

  std::error_code ec;
  std::filesystem::create_directories(log_dir_, ec);
  if (ec) {
    const std::string msg =
      std::format("[logger] FileSink disabled: create_directories failed dir={} ec={}", log_dir_.string(), ec.message());
    WarnOnce(msg);
    enabled_ = false;
    return false;
  }

  const std::filesystem::path first = MakeLogFilePath(log_dir_, cfg_);

  constexpr int kMaxAttempts = 100;
  for (int attempt = 0; attempt <= kMaxAttempts; ++attempt) {
    if (attempt == 0) {
      log_file_ = first;
    } else {
      const std::string name = first.stem().string() + std::format("_{}", attempt) + first.extension().string();
      log_file_ = log_dir_ / name;
    }

    auto out = std::make_unique<std::ofstream>(log_file_, std::ios::binary | std::ios::out | std::ios::app);
    if (out->is_open() && out->good()) {
      stream_ = std::move(out);
      enabled_ = true;
      FrameworkBootstrapLog(std::format("[logger] FileSink opened: {}", log_file_.string()));
      return true;
    }
  }

  const std::string msg = std::format(
    "[logger] FileSink disabled: failed to open log file in dir={} (mode={})", log_dir_.string(), ToString(cfg_.file_path_mode));
  WarnOnce(msg);
  enabled_ = false;
  return false;
}

void FileSink::OnLog(const LogEntry& entry) {
  if (!EnsureOpened()) {
    return;
  }

  std::lock_guard<std::mutex> lock(open_mutex_);
  if (!stream_ || !stream_->good()) {
    return;
  }

  const char* level = nullptr;
  switch (entry.level) {
    case LogLevel::Trace:
      level = "Trace";
      break;
    case LogLevel::Debug:
      level = "Debug";
      break;
    case LogLevel::Info:
      level = "Info";
      break;
    case LogLevel::Warn:
      level = "Warn";
      break;
    case LogLevel::Error:
      level = "Error";
      break;
    case LogLevel::Fatal:
      level = "Fatal";
      break;
  }

  const char* category = nullptr;
  switch (entry.category) {
    case LogCategory::Core:
      category = "Core";
      break;
    case LogCategory::Graphic:
      category = "Graphic";
      break;
    case LogCategory::Resource:
      category = "Resource";
      break;
    case LogCategory::Game:
      category = "Game";
      break;
    case LogCategory::UI:
      category = "UI";
      break;
    case LogCategory::Editor:
      category = "Editor";
      break;
    case LogCategory::Validation:
      category = "Validation";
      break;
  }

  const std::string line = std::format("[{}][{}] {} ({}:{})\n", level, category, entry.message, entry.loc.file_name(), entry.loc.line());
  (*stream_) << line;

  if (!stream_->good()) {
    enabled_ = false;
    stream_.reset();
    WarnOnce("[logger] FileSink disabled: write failed");
  }
}

void FileSink::Flush() {
  std::lock_guard<std::mutex> lock(open_mutex_);
  if (!enabled_ || !stream_) {
    return;
  }
  stream_->flush();
}
