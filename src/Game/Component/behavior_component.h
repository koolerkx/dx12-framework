#pragma once
#include "Component/component.h"

// Semantic base for non-rendering logic components (user scripts, controllers)
template <typename Derived>
class BehaviorComponent : public Component<Derived> {
 public:
  using Component<Derived>::Component;
};
