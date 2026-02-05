/**
 * @file EventBus.hpp
 * @brief Type-safe, thread-safe event bus with async support and cancellation.
 * @details Provides pub-sub event system with compile-time type safety, RAII handles,
 *          stable ID-based subscriptions, and integration with ThreadPool for async dispatch.
 *
 * Key Features:
 * - Compile-time type safety (no std::any, no runtime casting)
 * - Sync/Async emit with optional cancellation
 * - RAII EventHandle for automatic cleanup
 * - Thread-safe handler storage with unique_lock + snapshot pattern
 * - ID-based subscriptions (unsubscribe is O(1) and doesn't invalidate other handles)
 *
 * @code{.cpp}
 * struct PlayerDamagedEvent : Event<PlayerDamagedEvent> {
 *   static constexpr std::string_view EventName = "player.damaged";
 *   int player_id;
 *   float damage;
 * };
 *
 * ThreadPool pool(4);
 * auto bus = std::make_shared<EventBus>(pool);
 *
 * auto handle = bus->Subscribe<PlayerDamagedEvent>([](const PlayerDamagedEvent& event) {
 *     std::cout << "Player " << event.player_id << " took " << event.damage << " damage\n";
 * });
 *
 * bus->Emit(PlayerDamagedEvent{.player_id = 1, .damage = 25.0f});  // Sync
 * bus->EmitAsync(PlayerDamagedEvent{.player_id = 1, .damage = 30.0f});  // Async
 *
 * handle.Unsubscribe();  // Manual cleanup (auto on destruction)
 * @endcode
 */

#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "cancellation_token.hpp"
#include "event.hpp"
#include "subject_id.hpp"
#include "task.hpp"
#include "task_extensions.hpp"
#include "thread_pool.hpp"

class EventBus;

class EventHandle {
 public:
  EventHandle(std::weak_ptr<EventBus> bus, std::type_index event_type, uint64_t handler_id, std::optional<SubjectID> target = std::nullopt)
      : bus_(std::move(bus)), event_type_(event_type), handler_id_(handler_id), target_(target) {
  }

  ~EventHandle() {
    Unsubscribe();
  }

  void Unsubscribe();

  EventHandle(EventHandle&&) = default;
  EventHandle& operator=(EventHandle&&) = default;

  EventHandle(const EventHandle&) = delete;
  EventHandle& operator=(const EventHandle&) = delete;

 private:
  std::weak_ptr<EventBus> bus_;
  std::type_index event_type_;
  uint64_t handler_id_;
  std::optional<SubjectID> target_;
  bool unsubscribed_{false};
};

class EventBus : public std::enable_shared_from_this<EventBus> {
 public:
  explicit EventBus(ThreadPool& pool) : pool_(pool) {
  }

  template <typename E>
    requires EventType<E>
  void Emit(const E& event) {
    // Take the registered handler
    std::type_index type_id(typeid(E));
    std::vector<TypeErasedHandler> handlers_snapshot;  // prevent long lock holds and potential deadlocks

    {
      std::unique_lock<std::mutex> lock(handlers_mutex_);
      auto event_it = event_handlers_.find(type_id);
      if (event_it != event_handlers_.end()) {
        handlers_snapshot.reserve(event_it->second.size());
        for (const auto& [id, handler] : event_it->second) {
          handlers_snapshot.push_back(handler);  // copy assignment
        }
      }
    }

    // Execute the registered handler
    for (auto& handler : handlers_snapshot) {
      try {
        handler(&event);
      } catch (const std::exception&) {
      }
    }
  }

  template <typename E>
    requires EventType<E>
  void EmitAsync(const E& event) {
    // Take the registered handler
    std::type_index type_id(typeid(E));
    std::vector<TypeErasedHandler> handlers_snapshot;  // prevent long lock holds and potential deadlocks

    {
      std::unique_lock<std::mutex> lock(handlers_mutex_);
      auto event_it = event_handlers_.find(type_id);
      if (event_it != event_handlers_.end()) {
        handlers_snapshot.reserve(event_it->second.size());
        for (const auto& [id, handler] : event_it->second) {
          handlers_snapshot.push_back(handler);  // copy assignment
        }
      }
    }

    // Execute the registered handler
    auto event_copy = std::make_shared<E>(event);  // prevent access violation when leaving the scope
    for (auto& handler : handlers_snapshot) {
      pool_.Enqueue([handler, event_copy]() {
        try {
          handler(event_copy.get());
        } catch (const std::exception&) {
        }
      });
    }
  }

  template <typename E>
    requires EventType<E>
  void EmitAsync(const E& event, CancellationTokenPtr token) {
    if (token && token->IsCancelled()) {
      return;
    }

    // Take the registered handler
    std::type_index type_id(typeid(E));
    std::vector<TypeErasedHandler> handlers_snapshot;  // prevent long lock holds and potential deadlocks

    {
      std::unique_lock<std::mutex> lock(handlers_mutex_);
      auto event_it = event_handlers_.find(type_id);
      if (event_it != event_handlers_.end()) {
        handlers_snapshot.reserve(event_it->second.size());
        for (const auto& [id, handler] : event_it->second) {
          handlers_snapshot.push_back(handler);  // copy assignment
        }
      }
    }

    // Execute the registered handler
    auto event_copy = std::make_shared<E>(event);  // prevent access violation when leaving the scope
    for (auto& handler : handlers_snapshot) {
      if (token && token->IsCancelled()) {
        break;
      }

      pool_.Enqueue([handler, event_copy, token]() {
        if (token && token->IsCancelled()) {
          return;
        }
        try {
          handler(event_copy.get());
        } catch (const std::exception&) {
        }
      });
    }
  }

  template <typename E>
    requires EventType<E>
  void EmitTargeted(const E& event, SubjectID target) {
    std::type_index type_id(typeid(E));
    std::vector<TypeErasedHandler> handlers_snapshot;

    {
      std::unique_lock<std::mutex> lock(handlers_mutex_);
      auto event_it = targeted_handlers_.find(type_id);
      if (event_it != targeted_handlers_.end()) {
        auto target_it = event_it->second.find(target);
        if (target_it != event_it->second.end()) {
          handlers_snapshot.reserve(target_it->second.size());
          for (const auto& [id, handler] : target_it->second) {
            handlers_snapshot.push_back(handler);
          }
        }
      }
    }

    for (auto& handler : handlers_snapshot) {
      try {
        handler(&event);
      } catch (const std::exception&) {
      }
    }
  }

  template <typename E>
    requires EventType<E>
  void EmitTargetedAsync(const E& event, SubjectID target) {
    std::type_index type_id(typeid(E));
    std::vector<TypeErasedHandler> handlers_snapshot;

    {
      std::unique_lock<std::mutex> lock(handlers_mutex_);
      auto event_it = targeted_handlers_.find(type_id);
      if (event_it != targeted_handlers_.end()) {
        auto target_it = event_it->second.find(target);
        if (target_it != event_it->second.end()) {
          handlers_snapshot.reserve(target_it->second.size());
          for (const auto& [id, handler] : target_it->second) {
            handlers_snapshot.push_back(handler);
          }
        }
      }
    }

    auto event_copy = std::make_shared<E>(event);
    for (auto& handler : handlers_snapshot) {
      pool_.Enqueue([handler, event_copy]() {
        try {
          handler(event_copy.get());
        } catch (const std::exception&) {
        }
      });
    }
  }

  template <typename E>
    requires EventType<E>
  void EmitTargetedAsync(const E& event, SubjectID target, CancellationTokenPtr token) {
    if (token && token->IsCancelled()) {
      return;
    }

    std::type_index type_id(typeid(E));
    std::vector<TypeErasedHandler> handlers_snapshot;

    {
      std::unique_lock<std::mutex> lock(handlers_mutex_);
      auto event_it = targeted_handlers_.find(type_id);
      if (event_it != targeted_handlers_.end()) {
        auto target_it = event_it->second.find(target);
        if (target_it != event_it->second.end()) {
          handlers_snapshot.reserve(target_it->second.size());
          for (const auto& [id, handler] : target_it->second) {
            handlers_snapshot.push_back(handler);
          }
        }
      }
    }

    auto event_copy = std::make_shared<E>(event);
    for (auto& handler : handlers_snapshot) {
      if (token && token->IsCancelled()) {
        break;
      }

      pool_.Enqueue([handler, event_copy, token]() {
        if (token && token->IsCancelled()) {
          return;
        }
        try {
          handler(event_copy.get());
        } catch (const std::exception&) {
        }
      });
    }
  }

  /**
   * @brief Publishes event asynchronously and returns awaitable task
   */
  template <typename E>
    requires EventType<E>
  std::shared_ptr<Task<void>> PublishAsync(const E& event) {
    return PublishAsyncImpl(event, nullptr);
  }

  /**
   * @brief Publishes event asynchronously with cancellation support
   */
  template <typename E>
    requires EventType<E>
  std::shared_ptr<Task<void>> PublishAsync(const E& event, CancellationTokenPtr token) {
    return PublishAsyncImpl(event, token);
  }

  template <typename E>
    requires EventType<E>
  EventHandle Subscribe(std::function<void(const E&)> handler) {
    std::type_index type_id(typeid(E));

    auto type_erased_handler = [handler](const void* data) { handler(*static_cast<const E*>(data)); };

    uint64_t handler_id;
    {
      std::unique_lock<std::mutex> lock(handlers_mutex_);
      handler_id = next_handler_id_++;
      event_handlers_[type_id][handler_id] = std::move(type_erased_handler);
    }
    return EventHandle(weak_from_this(), type_id, handler_id);
  }

  template <typename E>
    requires EventType<E>
  EventHandle SubscribeTargeted(SubjectID target, std::function<void(const E&)> handler) {
    std::type_index type_id(typeid(E));

    auto type_erased_handler = [handler](const void* data) { handler(*static_cast<const E*>(data)); };

    uint64_t handler_id;
    {
      std::unique_lock<std::mutex> lock(handlers_mutex_);
      handler_id = next_handler_id_++;
      targeted_handlers_[type_id][target][handler_id] = std::move(type_erased_handler);
    }

    return EventHandle(weak_from_this(), type_id, handler_id, target);
  }

  EventBus(const EventBus&) = delete;
  EventBus& operator=(const EventBus&) = delete;

 private:
  friend class EventHandle;

  using TypeErasedHandler = std::function<void(const void*)>;

  void Unsubscribe(std::type_index event_type, uint64_t handler_id);
  void UnsubscribeTargeted(std::type_index event_type, SubjectID target, uint64_t handler_id);

  template <typename E>
    requires EventType<E>
  std::shared_ptr<Task<void>> PublishAsyncImpl(const E& event, CancellationTokenPtr token) {
    if (token && token->IsCancelled()) {
      auto cancelled_task = std::make_shared<Task<void>>([]() { throw TaskCancelledException(); });
      cancelled_task->TrySchedule(pool_);
      return cancelled_task;
    }

    std::type_index type_id(typeid(E));
    std::vector<TypeErasedHandler> handlers_snapshot;

    {
      std::unique_lock<std::mutex> lock(handlers_mutex_);
      auto event_it = event_handlers_.find(type_id);
      if (event_it != event_handlers_.end()) {
        handlers_snapshot.reserve(event_it->second.size());
        for (const auto& [id, handler] : event_it->second) {
          handlers_snapshot.push_back(handler);
        }
      }
    }

    if (handlers_snapshot.empty()) {
      auto empty_task = std::make_shared<Task<void>>([]() {});
      empty_task->TrySchedule(pool_);
      return empty_task;
    }

    auto event_copy = std::make_shared<E>(event);
    std::vector<std::shared_ptr<Task<void>>> handler_tasks;
    handler_tasks.reserve(handlers_snapshot.size());

    for (auto& handler : handlers_snapshot) {
      auto task = std::make_shared<Task<void>>([handler, event_copy, token]() {
        if (token && token->IsCancelled()) {
          return;
        }
        handler(event_copy.get());
      });
      handler_tasks.push_back(task);
    }

    if (token) {
      return WhenAllWithCancellation(pool_, handler_tasks, token);
    } else {
      return WhenAll(pool_, handler_tasks);
    }
  }

  ThreadPool& pool_;
  uint64_t next_handler_id_{0};
  std::mutex handlers_mutex_;
  std::unordered_map<std::type_index, std::unordered_map<uint64_t, TypeErasedHandler>> event_handlers_;
  std::unordered_map<std::type_index, std::unordered_map<SubjectID, std::unordered_map<uint64_t, TypeErasedHandler>>> targeted_handlers_;
};
