#include "Scenes/city_scene/currency_component.h"

#include "Framework/Event/event_bus.hpp"
#include "Framework/Logging/logger.h"
#include "Scenes/city_scene/city_scene_config.h"
#include "Scenes/city_scene/city_scene_events.h"
#include "game_context.h"

CurrencyComponent::CurrencyComponent(GameObject* owner, const Props& /*props*/) : BehaviorComponent(owner) {
}

void CurrencyComponent::OnStart() {
  const CitySceneConfig::GoldConfig gold_cfg;
  gold_ = gold_cfg.initial_gold;
  Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(), "[Gold] Initialized with {} gold", gold_);
  GetContext()->GetEventBus()->Emit(GoldChangedEvent{.gold = gold_});
}

void CurrencyComponent::AddGold(int amount) {
  gold_ += amount;
  Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(), "[Gold] +{} gold (total: {})", amount, gold_);
  GetContext()->GetEventBus()->Emit(GoldChangedEvent{.gold = gold_});
}

bool CurrencyComponent::TrySpendGold(int amount) {
  if (gold_ < amount) {
    Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(), "[Gold] Not enough gold: need {}, have {}", amount, gold_);
    GetContext()->GetEventBus()->Emit(InsufficientGoldEvent{.required = amount, .available = gold_});
    return false;
  }
  gold_ -= amount;
  Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(), "[Gold] -{} gold (total: {})", amount, gold_);
  GetContext()->GetEventBus()->Emit(GoldChangedEvent{.gold = gold_});
  return true;
}
