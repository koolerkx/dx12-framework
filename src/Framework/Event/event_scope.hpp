/**
 * @file EventScope.hpp
 * @brief RAII container for managing event subscriptions with async UAF prevention.
 * EventScope prevents UAF (Use-After-Free) in async handlers by using shared
 * CancellationToken ownership.
 *
 * @note The only things that EventScope do is prevent memory issue.
 * Not purposing to interrupt already-executing handlers, use token inside handler for that
 *
 * @warning
 * DO NOT capture [this] in SubscribeAsync handlers. Use shared_from_this() instead
 * @code{.cpp}
 * // WRONG - UAF if EventScope destroyed before handler runs
 * scope_.SubscribeAsync<MyEvent>(bus, [this](auto& e) { DoSomething(); });
 *
 * // RIGHT - shared_ptr keeps object alive
 * auto self = shared_from_this();
 * scope_.SubscribeAsync<MyEvent>(bus, [self](auto& e) { self->DoSomething(); });
 * @endcode
 */

#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "cancellation_token.hpp"
#include "event.hpp"
#include "event_bus.hpp"

class EventScope {
 public:
  EventScope() : token_(MakeCancellationToken()) {
  }

  ~EventScope() {
    if (token_) {  // Cancel token before unsubscribing to prevent async UAF
      token_->Cancel();
    }
    std::lock_guard<std::mutex> lock(handles_mutex_);
    handles_.clear();
  }

  // Subscribe to event sync
  template <typename E>
    requires EventType<E>
  void Subscribe(EventBus& bus, std::function<void(const E&)> handler) {
    auto handle = bus.Subscribe<E>(std::move(handler));
    {
      std::lock_guard<std::mutex> lock(handles_mutex_);
      handles_.push_back(std::move(handle));
    }
  }

  // Subscribe to targeted event sync
  template <typename E>
    requires EventType<E>
  void Subscribe(EventBus& bus, SubjectID target, std::function<void(const E&)> handler) {
    auto handle = bus.SubscribeTargeted<E>(target, std::move(handler));
    {
      std::lock_guard<std::mutex> lock(handles_mutex_);
      handles_.push_back(std::move(handle));
    }
  }

  /**
   * @brief Subscribe to events async with UAF protection.
   * @warning Handler must NOT capture [this]. Use shared_from_this() instead.
   */
  template <typename E>
    requires EventType<E>
  void SubscribeAsync(EventBus& bus, std::function<void(const E&)> handler) {
    CancellationTokenPtr token = token_;

    auto safe_handler = [token, handler](const E& event) {
      if (token && token->IsCancelled()) {
        return;
      }
      handler(event);
    };

    auto handle = bus.Subscribe<E>(std::move(safe_handler));
    {
      std::lock_guard<std::mutex> lock(handles_mutex_);
      handles_.push_back(std::move(handle));
    }
  }

  /**
   * @brief Subscribe to events async with UAF protection.
   * @warning Handler must NOT capture [this]. Use shared_from_this() instead.
   */
  template <typename E>
    requires EventType<E>
  void SubscribeAsync(EventBus& bus, SubjectID target, std::function<void(const E&)> handler) {
    CancellationTokenPtr token = token_;

    auto safe_handler = [token, handler](const E& event) {
      if (token && token->IsCancelled()) {
        return;
      }
      handler(event);
    };

    auto handle = bus.SubscribeTargeted<E>(target, std::move(safe_handler));
    {
      std::lock_guard<std::mutex> lock(handles_mutex_);
      handles_.push_back(std::move(handle));
    }
  }

  void Cancel() {
    if (token_) {
      token_->Cancel();
    }
  }

  bool IsCancelled() const {
    return token_ && token_->IsCancelled();
  }

  CancellationTokenPtr GetToken() const {
    return token_;
  }

  EventScope(const EventScope&) = delete;
  EventScope& operator=(const EventScope&) = delete;

  EventScope(EventScope&&) = delete;
  EventScope& operator=(EventScope&&) = delete;

 private:
  CancellationTokenPtr token_;
  std::mutex handles_mutex_;
  std::vector<EventHandle> handles_;
};
