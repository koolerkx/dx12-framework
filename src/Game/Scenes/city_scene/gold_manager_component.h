#pragma once

#include "Component/behavior_component.h"
#include "Framework/Logging/logger.h"
#include "Scenes/city_scene/city_scene_config.h"

class GoldManagerComponent : public BehaviorComponent<GoldManagerComponent> {
 public:
  using BehaviorComponent::BehaviorComponent;

  void OnStart() override {
    const CitySceneConfig::GoldConfig cfg;
    gold_ = cfg.initial_gold;
    Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(),
      "[Gold] Initialized with {} gold", gold_);
  }

  void AddGold(int amount) {
    gold_ += amount;
    Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(),
      "[Gold] +{} gold (total: {})", amount, gold_);
  }

  bool TrySpendGold(int amount) {
    if (gold_ < amount) {
      Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(),
        "[Gold] Not enough gold: need {}, have {}", amount, gold_);
      return false;
    }
    gold_ -= amount;
    Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(),
      "[Gold] -{} gold (total: {})", amount, gold_);
    return true;
  }

  int GetGold() const { return gold_; }

 private:
  int gold_ = 0;
};
