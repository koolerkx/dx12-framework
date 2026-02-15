#pragma once

#include "Component/component.h"
#include "Framework/Math/Math.h"

class ColliderComponent : public IComponentBase {
 public:
  using IComponentBase::IComponentBase;

  virtual Math::AABB GetWorldBounds() const = 0;
};
