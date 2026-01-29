/**
@filename logger.cpp
@brief Core logging manager implementation. Owns global logger state, manages sinks, performs thread-safe log dispatch, filtering, and
flushing.
@author Kooler Fan
**/
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

namespace {
constexpr size_t QUEUE_BATCH_SIZE = 2048;
constexpr std::chrono::milliseconds WORKER_WAIT_MS(25);
}  // namespace

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
    State& s = GetState();
    std::vector<LogEntry> batch;
    batch.reserve(512);

    while (true) {
      {  // wait for batch full or timeout to send log request from queue to batch
        std::unique_lock<std::mutex> lock(s.queue_mutex);
        s.cv.wait_for(lock, WORKER_WAIT_MS, [&]() { return !s.queue.empty() || !s.running.load(); });

        batch.clear();
        while (!s.queue.empty() && batch.size() < QUEUE_BATCH_SIZE) {
          batch.push_back(std::move(s.queue.front()));
          s.queue.pop_front();
        }
      }

      if (!batch.empty()) {  // log the batch to sinks
        EmitBatchToSinks(s, batch);
      }

      {  // check running state
        std::unique_lock<std::mutex> lock(s.queue_mutex);
        if (!s.running.load(std::memory_order_acquire) && s.queue.empty() && batch.empty()) {
          break;
        }
      }
    }
  });

  state.initialized.store(true, std::memory_order_release);
}

void Logger::Init(std::vector<std::unique_ptr<ILogSink>> sinks) {
  LoggerConfig cfg;
  Init(std::move(cfg), std::move(sinks));
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
  drained.reserve(QUEUE_BATCH_SIZE);

  {
    std::scoped_lock lock(state.queue_mutex);
    while (!state.queue.empty() && drained.size() < QUEUE_BATCH_SIZE) {
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
    } else if (state.config.overflow_policy == OverflowPolicy::DropLowFirst) {
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

void Logger::Logv(LogLevel level, LogCategory category, std::string_view fmt, std::format_args args, const std::source_location& loc) {
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
