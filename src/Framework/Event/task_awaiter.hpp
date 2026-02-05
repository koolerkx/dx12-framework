/**
 * @file TaskAwaiter.hpp
 * @brief Coroutine awaiter for Task<T> and Task<void>.
 * @details Provides awaitable adapters that resume coroutines when underlying tasks complete and handle exceptions/results.
 * @note Use within coroutines to await tasks; resume happens via a resumption Task
 *
 * @code{.cpp}
 * TaskAwaiter<void> awaiter{task, pool};
 * co_await awaiter;
 * @endcode
 */

#pragma once

#include <coroutine>

#include "task.hpp"

// Primary template for TaskAwaiter<T> - returns the task's result
template <typename T>
struct TaskAwaiter {
  std::shared_ptr<Task<T>> task;
  ThreadPool& pool;

  bool await_ready() {
    return task->IsDone();
  }

  void await_suspend(std::coroutine_handle<> awaiting_coro) {
    auto resumption = std::make_shared<Task<void>>([awaiting_coro]() { awaiting_coro.resume(); });

    task->Finally(resumption);

    if (task->IsDone()) {
      resumption->OnPredecessorFinished(pool);
    } else {
      task->TrySchedule(pool);
    }
  }

  T await_resume() {
    return task->GetResult();
  }
};

// Specialization for TaskAwaiter<void> - maintains existing behavior
template <>
struct TaskAwaiter<void> {
  std::shared_ptr<Task<void>> task;
  ThreadPool& pool;

  bool await_ready() {
    return task->IsDone();
  }

  void await_suspend(std::coroutine_handle<> awaiting_coro) {
    auto resumption = std::make_shared<Task<void>>([awaiting_coro]() { awaiting_coro.resume(); });

    task->Finally(resumption);

    if (task->IsDone()) {
      resumption->OnPredecessorFinished(pool);
    } else {
      task->TrySchedule(pool);
    }
  }

  void await_resume() {
    if (task->exception_) {
      std::rethrow_exception(task->exception_);
    }
  }
};
