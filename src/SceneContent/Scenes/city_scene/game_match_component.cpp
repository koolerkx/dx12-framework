#include "Scenes/city_scene/game_match_component.h"

#include "Framework/Event/event_bus.hpp"
#include "Framework/Logging/logger.h"
#include "Scenes/city_scene/wave_controller_component.h"
#include "game_context.h"
#include "game_object.h"
#include "scene_events.h"

GameMatchComponent::GameMatchComponent(GameObject* owner, const Props& /*props*/) : BehaviorComponent(owner) {
}

void GameMatchComponent::SetGameOver() {
  game_state_ = GameState::GameOver;

  auto* wave_ctrl = GetOwner()->GetComponent<WaveControllerComponent>();
  int wave = wave_ctrl ? wave_ctrl->GetCurrentWave() : 0;

  Logger::LogFormat(
    LogLevel::Info, LogCategory::Game, Logger::Here(), "[GameOver] Player base destroyed! Wave: {}, Kills: {}", wave, kill_count_);
  GetContext()->GetEventBus()->Emit(GameOverEvent{.wave = wave, .kill_count = kill_count_});
}

void GameMatchComponent::IncrementKillCount() {
  ++kill_count_;
}
