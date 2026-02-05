/**
 * @file ThreadPool.hpp
 * @brief Simple fixed-size thread pool for enqueuing work.
 * @details Creates worker threads that process tasks from an internal queue and supports graceful shutdown in destructor.
 * @note Default thread count is `hardware_concurrency() - 1` (at least one)
 *
 * @code{.cpp}
 * ThreadPool pool(4);
 * pool.Enqueue([](){});
 * @endcode
 */

#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
 public:
  explicit ThreadPool(size_t threads = GetDefaultThreadCount()) {
    for (size_t i = 0; i < threads; ++i) {
      workers.emplace_back(std::thread([this] {
        while (true) {
          std::function<void()> task;
          {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this] { return stop || !tasks.empty(); });

            // Handling thread pool shutdown
            if (stop && tasks.empty()) {
              return;
            }

            task = std::move(tasks.front());
            tasks.pop();
          }
          task();
        }
      }));
    }
  }

  void Enqueue(std::function<void()> task) {
    {
      std::unique_lock<std::mutex> lock(queueMutex);
      tasks.emplace(std::move(task));
    }
    condition.notify_one();
  }

  ~ThreadPool() {
    {
      std::unique_lock<std::mutex> lock(queueMutex);
      stop = true;
    }
    condition.notify_all();
    for (std::thread& worker : workers) {
      worker.join();
    }
  }

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

 private:
  static size_t GetDefaultThreadCount() {
    auto core = std::thread::hardware_concurrency();
    if (core == 0) return 1;
    return (std::max)(size_t{1}, static_cast<size_t>(core - 1));
  }

  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;
  std::mutex queueMutex;
  std::condition_variable condition;
  bool stop = false;
};
