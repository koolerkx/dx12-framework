/**
 * @file CancellationToken.hpp
 * @brief Lightweight cancellation token to signal and observe cancellation.
 * @details Provides cancellation signaling, callback registration, and exception support via TaskCancelledException.
 * @note Use callbacks to react to cancellation
 */
#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

class TaskCancelledException : public std::exception {
 public:
  const char* what() const noexcept override {
    return "Task was cancelled";
  }
};

class CancellationToken {
 public:
  CancellationToken() = default;

  void Cancel() {
    if (!is_cancelled_.exchange(true, std::memory_order_acq_rel)) {
      std::lock_guard<std::mutex> lock(callbacks_mutex_);
      for (auto& callback : callbacks_) {
        if (callback) {
          callback();
        }
      }
      callbacks_.clear();
    }
  }

  bool IsCancelled() const {
    return is_cancelled_.load(std::memory_order_acquire);
  }

  void ThrowIfCancelled() const {
    if (IsCancelled()) {
      throw TaskCancelledException();
    }
  }

  void RegisterCallback(std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    if (IsCancelled()) {
      callback();
    } else {
      callbacks_.push_back(std::move(callback));
    }
  }

 private:
  std::atomic<bool> is_cancelled_{false};
  std::mutex callbacks_mutex_;
  std::vector<std::function<void()>> callbacks_;
};

using CancellationTokenPtr = std::shared_ptr<CancellationToken>;

inline CancellationTokenPtr MakeCancellationToken() {
  return std::make_shared<CancellationToken>();
}
