/**
 * @file Event.hpp
 * @brief Type-safe event base class using CRTP pattern.
 * @details Provides compile-time type safety for EventBus through CRTP.
 *          Event structs inherit from Event<Derived> and define EventName.
 *
 * @code{.cpp}
 * struct PlayerDamagedEvent : Event<PlayerDamagedEvent> {
 *   static constexpr std::string_view EventName = "player.damaged";
 *   int player_id;
 *   float damage;
 * };
 *
 * bus->Emit(PlayerDamagedEvent{.player_id = 1, .damage = 25.0f});
 * bus->Subscribe<PlayerDamagedEvent>([](const PlayerDamagedEvent& event) { ... });
 * @endcode
 */
#pragma once

#include <string_view>
#include <type_traits>

template <typename Derived>
struct Event {
  static constexpr std::string_view GetEventName() {
    return Derived::EventName;
  }
};

template <typename T>
concept EventType = requires {
  { T::EventName } -> std::convertible_to<std::string_view>;
} && std::is_base_of_v<Event<T>, T>;
