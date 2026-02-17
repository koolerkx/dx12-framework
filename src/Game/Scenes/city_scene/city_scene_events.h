#pragma once

#include "Framework/Event/event.hpp"

struct GoldChangedEvent : Event<GoldChangedEvent> {
  static constexpr std::string_view EventName = "city.gold_changed";
  int gold;
};

struct HealthChangedEvent : Event<HealthChangedEvent> {
  static constexpr std::string_view EventName = "city.health_changed";
  int health;
};

struct WaveStartEvent : Event<WaveStartEvent> {
  static constexpr std::string_view EventName = "city.wave_start";
  int wave;
};

struct WaveCountdownEvent : Event<WaveCountdownEvent> {
  static constexpr std::string_view EventName = "city.wave_countdown";
  float seconds_remaining;
};

struct InsufficientGoldEvent : Event<InsufficientGoldEvent> {
  static constexpr std::string_view EventName = "city.insufficient_gold";
  int required;
  int available;
};

struct EnemyReachedBaseEvent : Event<EnemyReachedBaseEvent> {
  static constexpr std::string_view EventName = "city.enemy_reached_base";
};
