#include "component.h"
#include "game_object.h"

GameContext* IComponentBase::GetContext() const {
  return owner_ ? owner_->GetContext() : nullptr;
}

int IComponentBase::InheritParentUILayerId() const {
  auto* parent = owner_->GetParent();
  if (!parent) return 0;
  for (auto& comp : parent->GetComponents()) {
    if (auto id = comp->GetUILayerId()) return *id - 1;
  }
  return 0;
}
