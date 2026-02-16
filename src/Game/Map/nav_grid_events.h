#pragma once

#include "Framework/Event/event.hpp"
#include "Framework/Math/Math.h"

struct NavGridChangedEvent : Event<NavGridChangedEvent> {
  static constexpr std::string_view EventName = "nav_grid.changed";
  Math::AABB affected_area;
};
