/**
 * @file TaskExtensions.hpp
 * @brief Extension helpers: cancellation, timeout, polling variants, and task composition.
 * @details Provides WithCancellation, WithTimeout, WithPollingCancellation helpers to adapt work into cancellable tasks,
 *          and WhenAll for aggregating multiple tasks.
 * @note WithTimeout returns an out CancellationTokenPtr if requested
 *
 * @code{.cpp}
 * auto t = WithCancellation([]() { return 1; }, MakeCancellationToken());
 * auto aggregated = WhenAll(pool, {task1, task2, task3});
 * @endcode
 */
#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include "cancellation_token.hpp"
#include "task.hpp"
#include "thread_pool.hpp"
#include "timeout_guard.hpp"

template <typename T>
std::shared_ptr<Task<T>> WithCancellation(std::function<T()> work, CancellationTokenPtr token) {
  return std::make_shared<Task<T>>([work = std::move(work), token]() -> T {
    token->ThrowIfCancelled();
    return work();
  });
}

template <>
inline std::shared_ptr<Task<void>> WithCancellation(std::function<void()> work, CancellationTokenPtr token) {
  return std::make_shared<Task<void>>([work = std::move(work), token]() {
    token->ThrowIfCancelled();
    work();
  });
}

template <typename T, typename Rep, typename Period>
std::shared_ptr<Task<T>> WithTimeout(
  std::function<T()> work, std::chrono::duration<Rep, Period> timeout, CancellationTokenPtr* out_token = nullptr) {
  auto token = MakeCancellationToken();
  if (out_token) {
    *out_token = token;
  }

  auto task = std::make_shared<Task<T>>([work = std::move(work), token, timeout]() -> T {
    TimeoutGuard guard(token, timeout);
    token->ThrowIfCancelled();
    return work();
  });

  return task;
}

template <typename Rep, typename Period>
std::shared_ptr<Task<void>> WithTimeout(
  std::function<void()> work, std::chrono::duration<Rep, Period> timeout, CancellationTokenPtr* out_token = nullptr) {
  auto token = MakeCancellationToken();
  if (out_token) {
    *out_token = token;
  }

  auto task = std::make_shared<Task<void>>([work = std::move(work), token, timeout]() {
    TimeoutGuard guard(token, timeout);
    token->ThrowIfCancelled();
    work();
  });

  return task;
}

template <typename T>
std::shared_ptr<Task<T>> WithPollingCancellation(std::function<T(CancellationTokenPtr)> work, CancellationTokenPtr token) {
  return std::make_shared<Task<T>>([work = std::move(work), token]() -> T { return work(token); });
}

template <>
inline std::shared_ptr<Task<void>> WithPollingCancellation(std::function<void(CancellationTokenPtr)> work, CancellationTokenPtr token) {
  return std::make_shared<Task<void>>([work = std::move(work), token]() { work(token); });
}

inline std::shared_ptr<Task<void>> WhenAll(ThreadPool& pool, std::vector<std::shared_ptr<Task<void>>> tasks) {
  if (tasks.empty()) {
    auto empty_task = std::make_shared<Task<void>>([]() {});
    empty_task->TrySchedule(pool);
    return empty_task;
  }

  auto aggregate_task = std::make_shared<Task<void>>([]() {});

  for (auto& task : tasks) {
    task->Then(aggregate_task);
  }

  for (auto& task : tasks) {
    task->TrySchedule(pool);
  }

  return aggregate_task;
}

inline std::shared_ptr<Task<void>> WhenAllWithCancellation(
  ThreadPool& pool, std::vector<std::shared_ptr<Task<void>>> tasks, CancellationTokenPtr token) {
  if (token && token->IsCancelled()) {
    auto cancelled_task = std::make_shared<Task<void>>([]() { throw TaskCancelledException(); });
    cancelled_task->TrySchedule(pool);
    return cancelled_task;
  }

  if (tasks.empty()) {
    auto empty_task = std::make_shared<Task<void>>([]() {});
    empty_task->TrySchedule(pool);
    return empty_task;
  }

  auto aggregate_task = std::make_shared<Task<void>>([token]() {
    if (token && token->IsCancelled()) {
      throw TaskCancelledException();
    }
  });

  for (auto& task : tasks) {
    task->Then(aggregate_task);
  }

  for (auto& task : tasks) {
    task->TrySchedule(pool);
  }

  return aggregate_task;
}
