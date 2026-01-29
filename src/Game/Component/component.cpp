#include "component.h"
#include "game_object.h"

GameContext* IComponentBase::GetContext() const {
  return owner_ ? owner_->GetContext() : nullptr;
}
