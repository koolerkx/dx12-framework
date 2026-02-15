#pragma once

#include <vector>

#include "Framework/Math/Math.h"

class NavGrid;

struct PathResult {
  bool found = false;
  std::vector<Math::Vector2> waypoints;
};

class Pathfinder {
 public:
  static PathResult FindPath(const NavGrid& grid, Math::Vector2 start_world, Math::Vector2 goal_world, float node_size = 1.0f);
};
