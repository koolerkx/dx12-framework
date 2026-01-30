/**
 * @file logger.cpp
 * @author Kooler Fan
 * @brief Core logging manager implementation
 *
 * Behavior:
 * 1. new message enqueued -> notify worker (state.cv.notify_one())
 * 2. worker wakes on notify OR after LoggerConfig.worker_wait_ms timeout
 * 3. worker drains up to LoggerConfig.batch_size messages and calls sinks via EmitBatchToSinks()
 *
 * Flush(): drain up to QUEUE_BATCH_SIZE and then call FlushSinks() (force write)
 * EmitDirectMinimal(): bypass queue and log+flush directly (used for panic/fatal)
 */
#include "Framework/Logging/logger.h"

#include <array>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <format>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

#include "Framework/Error/framework_bootstrap_log.h"

// Batch size and worker wait are configured via LoggerConfig (batch_size, worker_wait_ms)
// See LoggerConfig in Framework/Logging/logger_config.h for defaults and customization

struct Logger::State {
  std::mutex sinks_mutex;
  std::vector<std::unique_ptr<ILogSink>> sinks;

  std::mutex queue_mutex;
  std::condition_variable cv;
  std::deque<LogEntry> queue;
  size_t queue_capacity = 8192;

  std::atomic<bool> running{false};
  std::thread worker;

  std::atomic<bool> initialized{false};
  LogLevel min_level = LogLevel::Trace;

  LoggerConfig config = {};

  std::atomic<bool> panic{false};
  std::atomic_flag reentrancy_guard = ATOMIC_FLAG_INIT;

  std::atomic<uint64_t> dropped_total{0};
  std::array<std::atomic<uint64_t>, 6> dropped_by_level{};
};

Logger::State& Logger::GetState() {
  static State state;
  return state;
}

namespace {

uint8_t ToIndex(LogLevel level) {
  return static_cast<uint8_t>(level);
}

bool IsPowerOfTwo(uint64_t x) {
  return x != 0 && ((x & (x - 1)) == 0);
}

[[nodiscard]] bool DropOneLowPriority(std::deque<LogEntry>& queue) {
  for (auto it = queue.rbegin(); it != queue.rend(); ++it) {
    if (it->level == LogLevel::Trace || it->level == LogLevel::Debug) {
      queue.erase((it + 1).base());
      return true;
    }
  }
  for (auto it = queue.rbegin(); it != queue.rend(); ++it) {
    if (it->level == LogLevel::Info) {
      queue.erase((it + 1).base());
      return true;
    }
  }
  for (auto it = queue.rbegin(); it != queue.rend(); ++it) {
    if (it->level == LogLevel::Warn) {
      queue.erase((it + 1).base());
      return true;
    }
  }
  return false;
}

template <class StateT>
void EmitBatchToSinks(StateT& state, const std::vector<LogEntry>& batch) {
  std::scoped_lock lock(state.sinks_mutex);
  if (!state.initialized.load(std::memory_order_acquire)) {
    return;
  }

  for (const auto& entry : batch) {
    for (auto& sink : state.sinks) {
      if (sink) {
        sink->OnLog(entry);
      }
    }
  }
}

template <class StateT>
void FlushSinks(StateT& state) {
  std::scoped_lock lock(state.sinks_mutex);
  for (auto& sink : state.sinks) {
    if (sink) {
      sink->Flush();
    }
  }
}

}  // namespace

void Logger::EnterPanic() noexcept {
  GetState().panic.store(true, std::memory_order_release);
}

void Logger::Init(LoggerConfig cfg, std::vector<std::unique_ptr<ILogSink>> sinks) {
  State& state = GetState();
  {  // Load Config
    std::scoped_lock lock(state.sinks_mutex);
    state.config = std::move(cfg);
    state.queue_capacity = state.config.queue_capacity;

    state.sinks = std::move(sinks);
  }

  state.running.store(true, std::memory_order_release);

  state.worker = std::thread([]() {
    State& state = GetState();
    std::vector<LogEntry> batch;
    batch.reserve(512);

    while (true) {
      {  // wait for batch full or timeout to send log request from queue to batch
        std::unique_lock<std::mutex> lock(state.queue_mutex);
        // use configured wait duration and batch size from LoggerConfig
        auto wait_ms = state.config.worker_wait_ms;
        const size_t batch_size = state.config.batch_size;
        state.cv.wait_for(lock, wait_ms, [&]() { return !state.queue.empty() || !state.running.load(); });

        batch.clear();
        while (!state.queue.empty() && batch.size() < batch_size) {
          batch.push_back(std::move(state.queue.front()));
          state.queue.pop_front();
        }
      }

      if (!batch.empty()) {  // log the batch to sinks
        EmitBatchToSinks(state, batch);
      }

      {  // check running state
        std::unique_lock<std::mutex> lock(state.queue_mutex);
        if (!state.running.load(std::memory_order_acquire) && state.queue.empty() && batch.empty()) {
          break;
        }
      }
    }
  });

  state.initialized.store(true, std::memory_order_release);
}

void Logger::Init(std::vector<std::unique_ptr<ILogSink>> sinks) {
  LoggerConfig config;
  Init(std::move(config), std::move(sinks));
}

void Logger::Shutdown() {
  State& state = GetState();
  if (state.running.exchange(false)) {
    state.cv.notify_all();
    if (state.worker.joinable()) {
      state.worker.join();
    }
  }

  Flush();

  std::scoped_lock lock(state.sinks_mutex);
  state.initialized.store(false, std::memory_order_release);
  state.sinks.clear();
}

void Logger::Flush() {
  State& state = GetState();
  if (!state.initialized.load(std::memory_order_acquire)) {
    return;
  }

  std::vector<LogEntry> drained;
  const size_t batch_size = state.config.batch_size;
  drained.reserve(batch_size);

  {
    std::scoped_lock lock(state.queue_mutex);
    while (!state.queue.empty() && drained.size() < batch_size) {
      drained.push_back(std::move(state.queue.front()));
      state.queue.pop_front();
    }
  }

  if (!drained.empty()) {
    EmitBatchToSinks(state, drained);
  }

  FlushSinks(state);
}

bool Logger::IsInitialized() {
  return GetState().initialized.load(std::memory_order_acquire);
}

[[nodiscard]] bool Logger::IsEnabled(LogLevel level, [[maybe_unused]] LogCategory category) {
  const State& state = GetState();
  if (!state.initialized.load(std::memory_order_acquire)) {
    return false;
  }
  return static_cast<uint8_t>(level) >= static_cast<uint8_t>(state.min_level);
}

LoggerConfig Logger::GetConfig() {
  const State& state = GetState();
  return state.config;
}

void Logger::Log(LogLevel level, LogCategory category, std::string message, const std::source_location& loc) {
  if (!IsEnabled(level, category)) {
    return;
  }

  State& state = GetState();
  if (state.panic.load(std::memory_order_acquire)) {
    EmitDirectMinimal(level, category, message, loc);
    return;
  }

  LogEntry entry;
  entry.level = level;
  entry.category = category;
  entry.timestamp = std::chrono::system_clock::now();
  entry.loc = loc;
  entry.message = std::move(message);

  bool dropped = false;
  bool enqueued = false;
  {
    std::scoped_lock lock(state.queue_mutex);
    if (state.queue.size() < state.queue_capacity) {
      state.queue.push_back(std::move(entry));
      enqueued = true;
    } else if (state.config.overflow_policy == LoggerConfig::OverflowPolicy::DropLowFirst) {
      if (DropOneLowPriority(state.queue)) {
        state.queue.push_back(std::move(entry));
        enqueued = true;
        dropped = true;
      } else {
        dropped = true;
      }
    } else {
      dropped = true;
    }
  }

  if (enqueued) {
    state.cv.notify_one();
  }

  if (dropped) {
    const uint64_t total = state.dropped_total.fetch_add(1, std::memory_order_relaxed) + 1;
    state.dropped_by_level[ToIndex(level)].fetch_add(1, std::memory_order_relaxed);

    if (IsPowerOfTwo(total)) {
      FrameworkBootstrapLog(std::format("[logger] dropped {} messages due to full queue", total), loc);
    }
  }

  if (level == LogLevel::Fatal) {
    Flush();
  }
}

void Logger::LogFormatArgs(
  LogLevel level, LogCategory category, std::string_view fmt, std::format_args args, const std::source_location& loc) {
  if (!IsEnabled(level, category)) {
    return;
  }
  Log(level, category, std::vformat(fmt, args), loc);
}

void Logger::EmitDirectMinimal(LogLevel level, LogCategory category, std::string_view message, const std::source_location& loc) noexcept {
  State& state = GetState();
  if (state.reentrancy_guard.test_and_set(std::memory_order_acq_rel)) {
    return;
  }

  if (!state.initialized.load(std::memory_order_acquire)) {
    FrameworkBootstrapLog(message, loc);
    state.reentrancy_guard.clear(std::memory_order_release);
    return;
  }

  std::unique_lock<std::mutex> lock(state.sinks_mutex, std::try_to_lock);
  if (!lock.owns_lock()) {
    FrameworkBootstrapLog(message, loc);
    state.reentrancy_guard.clear(std::memory_order_release);
    return;
  }

  LogEntry entry;
  entry.level = level;
  entry.category = category;
  entry.timestamp = std::chrono::system_clock::now();
  entry.loc = loc;

  try {
    entry.message = std::string(message);
  } catch (...) {
    entry.message = "[logger] format error in panic message";
  }

  for (auto& sink : state.sinks) {
    if (sink) {
      sink->OnLog(entry);
      sink->Flush();
    }
  }

  state.reentrancy_guard.clear(std::memory_order_release);
}
