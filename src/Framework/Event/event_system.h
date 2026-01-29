/**
 * @file event_system.h
 * @brief Universal event system
 * @author Kooler Fan
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <mutex>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Event {

/**
 * @struct DispatchResult
 * @brief Return value from event listeners
 * - Continue: Let other listeners receive this event
 * - Consume: Stop propagation (no more listeners will see this event)
 */
enum class DispatchResult : uint8_t { Continue, Consume };

/**
 * @struct ListenerOptions
 * @brief Configuration for event subscriptions
 */
struct ListenerOptions {
  int32_t priority = 0;  // Higher priority = called first
  bool enabled = true;   // Can be toggled on/off
};

/**
 * @struct EventMetadata
 * @brief Internal tracking info for each event
 */
struct EventMetadata {
  uint64_t frame_index = 0;
  uint64_t timestamp_us = 0;
  uint32_t sequence = 0;
};

// Forward declaration
class EventBus;

/**
 * @class Subscription
 * @brief RAII handle for event listeners
 * @details Automatically unsubscribes when destroyed (React useEffect cleanup pattern)
 */
class Subscription {
 public:
  Subscription() = default;

  Subscription(const Subscription&) = delete;
  Subscription& operator=(const Subscription&) = delete;

  Subscription(Subscription&& other) noexcept : bus_(other.bus_), listener_id_(other.listener_id_) {
    other.bus_ = nullptr;
    other.listener_id_ = 0;
  }

  Subscription& operator=(Subscription&& other) noexcept {
    if (this != &other) {
      Unsubscribe();
      bus_ = other.bus_;
      listener_id_ = other.listener_id_;
      other.bus_ = nullptr;
      other.listener_id_ = 0;
    }
    return *this;
  }

  ~Subscription() {
    Unsubscribe();
  }

  bool IsValid() const {
    return bus_ != nullptr && listener_id_ != 0;
  }

  void Unsubscribe();

 private:
  friend class EventBus;

  Subscription(EventBus* bus, uint64_t id) : bus_(bus), listener_id_(id) {
  }

  EventBus* bus_ = nullptr;
  uint64_t listener_id_ = 0;
};

/**
 * @class EventBus
 * @brief Universal event dispatcher
 *
 * @code {.cpp}
 *   const sub = EventBus::Subscribe<MyEvent>([](const MyEvent& e) {
 *     console.log(e.data);
 *     return DispatchResult::Continue;
 *   });
 *
 *   EventBus::Emit(MyEvent{ data: 42 });  // Immediate dispatch
 *   EventBus::Post(MyEvent{ data: 42 });  // Buffered for end-of-frame
 *   EventBus::Flush();                    // Dispatch all posted events
 * @endcode
 */
class EventBus {
 public:
  /**
   * Subscribe to events of type E (React addEventListener pattern)
   * @tparam E Event type (must be copyable)
   * @param handler Callback function (event) => DispatchResult
   * @param options Priority, enabled state
   * @return Subscription token (RAII - auto unsubscribes on destruction)
   */
  template <typename E>
  static Subscription Subscribe(std::function<DispatchResult(const E&)> handler, ListenerOptions options = {}) {
    static_assert(std::is_copy_constructible_v<E>, "Event type must be copyable");

    std::lock_guard<std::mutex> lock(GetMutex());

    auto& listeners = GetListeners<E>();
    uint64_t id = ++GetNextListenerId();

    ListenerEntry entry;
    entry.id = id;
    entry.priority = options.priority;
    entry.enabled = options.enabled;
    entry.callback = [handler](const void* event) -> DispatchResult { return handler(*static_cast<const E*>(event)); };

    listeners.push_back(entry);

    // Sort by priority (descending: higher priority first)
    std::sort(listeners.begin(), listeners.end(), [](const ListenerEntry& a, const ListenerEntry& b) {
      if (a.priority != b.priority) return a.priority > b.priority;
      return a.id < b.id;  // Stable sort by registration order
    });

    return Subscription(&GetInstance(), id);
  }

  /**
   * Emit event immediately (synchronous dispatch)
   * Use this for critical events that must be handled NOW
   * @tparam E Event type
   * @param event Event instance
   */
  template <typename E>
  static void Emit(const E& event) {
    std::lock_guard<std::mutex> lock(GetMutex());

    auto& listeners = GetListeners<E>();

    for (auto& listener : listeners) {
      if (!listener.enabled) continue;

      DispatchResult result = listener.callback(&event);

      if (result == DispatchResult::Consume) {
        break;  // Stop propagation
      }
    }
  }

  /**
   * Post event for buffered dispatch (asynchronous, end-of-frame)
   * Use this for most events to avoid blocking game loop
   * @tparam E Event type
   * @param event Event instance
   */
  template <typename E>
  static void Post(const E& event) {
    std::lock_guard<std::mutex> lock(GetMutex());

    auto& buffer = GetEventBuffer();
    size_t seq = buffer.size();
    assert(seq <= (std::numeric_limits<uint32_t>::max)() && "Event buffer size exceeds uint32_t max");

    EventEntry entry{
      .type = std::type_index(typeid(E)),
      .sequence = static_cast<uint32_t>(seq),
      .event_data = std::make_shared<E>(event),
      .dispatcher = [](const std::shared_ptr<void>& data) { Emit(*std::static_pointer_cast<E>(data)); },
    };

    buffer.push_back(std::move(entry));
  }

  /**
   * Flush all posted events (call this once per frame)
   * Events are dispatched in the order they were posted
   */
  static void Flush() {
    // Copy buffer and clear it while holding lock
    std::vector<EventEntry> events_to_dispatch;
    {
      std::lock_guard<std::mutex> lock(GetMutex());
      auto& buffer = GetEventBuffer();
      events_to_dispatch = std::move(buffer);
      buffer.clear();
      ++GetFrameIndex();
    }

    // Dispatch events WITHOUT holding lock (avoid deadlock with Emit)
    for (auto& entry : events_to_dispatch) {
      entry.dispatcher(entry.event_data);
    }
  }

  /**
   * Clear all subscriptions and buffered events (for shutdown/scene change)
   */
  static void Clear() {
    std::lock_guard<std::mutex> lock(GetMutex());
    GetAllListeners().clear();
    GetEventBuffer().clear();
  }

  /**
   * Get current frame index
   */
  static uint64_t GetCurrentFrame() {
    return GetFrameIndex();
  }

  /**
   * Check if there are any subscriptions for event type E
   */
  template <typename E>
  static bool HasListeners() {
    std::lock_guard<std::mutex> lock(GetMutex());
    auto& listeners = GetListeners<E>();
    return !listeners.empty();
  }

 private:
  friend class Subscription;

  struct ListenerEntry {
    uint64_t id;
    int32_t priority;
    bool enabled;
    std::function<DispatchResult(const void*)> callback;
  };

  struct EventEntry {
    std::type_index type;
    uint32_t sequence;
    std::shared_ptr<void> event_data;
    std::function<void(const std::shared_ptr<void>&)> dispatcher;
  };

  template <typename E>
  static std::vector<ListenerEntry>& GetListeners() {
    return GetAllListeners()[std::type_index(typeid(E))];
  }

  static std::unordered_map<std::type_index, std::vector<ListenerEntry>>& GetAllListeners() {
    static std::unordered_map<std::type_index, std::vector<ListenerEntry>> listeners;
    return listeners;
  }

  static std::vector<EventEntry>& GetEventBuffer() {
    static std::vector<EventEntry> buffer;
    return buffer;
  }

  static uint64_t& GetNextListenerId() {
    static uint64_t next_id = 0;
    return next_id;
  }

  static uint64_t& GetFrameIndex() {
    static uint64_t frame = 0;
    return frame;
  }

  static std::mutex& GetMutex() {
    static std::mutex mutex;
    return mutex;
  }

  static EventBus& GetInstance() {
    static EventBus instance;
    return instance;
  }

  void UnsubscribeImpl(uint64_t listener_id) {
    std::lock_guard<std::mutex> lock(GetMutex());

    for (auto& [type, listeners] : GetAllListeners()) {
      auto it =
        std::remove_if(listeners.begin(), listeners.end(), [listener_id](const ListenerEntry& entry) { return entry.id == listener_id; });

      if (it != listeners.end()) {
        listeners.erase(it, listeners.end());
        break;
      }
    }
  }
};

inline void Subscription::Unsubscribe() {
  if (IsValid()) {
    bus_->UnsubscribeImpl(listener_id_);
    bus_ = nullptr;
    listener_id_ = 0;
  }
}

}  // namespace Event

using EventSystem = Event::EventBus;
