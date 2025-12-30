#pragma once
#include "Framework/Event/event_system.h"
#include "Framework/Input/input.h"
#include "Graphic/graphic.h"

class GameContext {
 public:
  GameContext() = default;

  void SetInputSystem(InputSystem* input) {
    input_ = input;
  }
  void SetGraphic(Graphic* graphic) {
    graphic_ = graphic;
  }
  void SetEventSystem(EventSystem* event_system) {
    event_system_ = event_system;
  }

  InputSystem* GetInput() const {
    return input_;
  }
  Graphic* GetGraphic() const {
    return graphic_;
  }
  EventSystem* GetEventSystem() const {
    return event_system_;
  }

 private:
  InputSystem* input_ = nullptr;
  Graphic* graphic_ = nullptr;
  EventSystem* event_system_ = nullptr;
};
