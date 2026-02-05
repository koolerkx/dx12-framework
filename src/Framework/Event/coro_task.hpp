/**
 * @file CoroTask.hpp
 * @brief Minimal coroutine wrapper that supports waiting and exception propagation.
 * @details Defines a simple coroutine task type that allows waiting for completion and rethrowing exceptions.
 * @note final_suspend notifies waiting threads
 */

#pragma once

#include <condition_variable>
#include <coroutine>
#include <exception>
#include <mutex>

template <typename T = void>
struct CoroTask {
  struct promise_type {
    std::exception_ptr exception_ = nullptr;
    std::mutex wait_mutex_;
    std::condition_variable wait_cv_;
    bool is_done_ = false;

    CoroTask get_return_object() {
      return CoroTask{std::coroutine_handle<promise_type>::from_promise(*this)};
    }

    std::suspend_never initial_suspend() {
      return {};
    }
    std::suspend_always final_suspend() noexcept {
      {
        std::lock_guard<std::mutex> lock(wait_mutex_);
        is_done_ = true;
      }
      wait_cv_.notify_all();
      return {};
    }

    void return_void() {
    }

    void unhandled_exception() {
      exception_ = std::current_exception();
    }
  };

  std::coroutine_handle<promise_type> handle;

  CoroTask() noexcept : handle(nullptr) {
  }
  CoroTask(std::coroutine_handle<promise_type> h) noexcept : handle(h) {
  }

  CoroTask(const CoroTask&) = delete;
  CoroTask& operator=(const CoroTask&) = delete;

  CoroTask(CoroTask&& other) noexcept : handle(other.handle) {
    other.handle = nullptr;
  }

  CoroTask& operator=(CoroTask&& other) noexcept {
    if (this != &other) {
      if (handle) {
        handle.destroy();
      }
      handle = other.handle;
      other.handle = nullptr;
    }
    return *this;
  }

  ~CoroTask() {
    if (handle) {
      handle.destroy();
    }
  }

  void Wait() {
    if (!handle) return;
    std::unique_lock<std::mutex> lock(handle.promise().wait_mutex_);
    handle.promise().wait_cv_.wait(lock, [this] { return handle.promise().is_done_; });
  }

  void rethrow_if_exception() const {
    if (handle && handle.promise().exception_) {
      std::rethrow_exception(handle.promise().exception_);
    }
  }
};
