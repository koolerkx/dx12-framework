#pragma once

#include <cstdint>

#include "Component/behavior_component.h"

enum class GameState : uint8_t {
  Playing,
  GameOver,
};

class GameMatchComponent : public BehaviorComponent<GameMatchComponent> {
 public:
  struct Props {};

  using BehaviorComponent::BehaviorComponent;
  GameMatchComponent(GameObject* owner, const Props& props);

  bool IsGameOver() const {
    return game_state_ == GameState::GameOver;
  }
  GameState GetGameState() const {
    return game_state_;
  }

  void SetGameOver();
  void IncrementKillCount();
  int GetKillCount() const {
    return kill_count_;
  }

 private:
  GameState game_state_ = GameState::Playing;
  int kill_count_ = 0;
};
