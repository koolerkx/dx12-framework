#pragma once

#include <functional>
#include <vector>

struct AStarResult {
  bool found = false;
  std::vector<int> path;
};

using BlockedCheck = std::function<bool(int col, int row)>;

AStarResult FindPathAStar(
  int grid_width, int grid_height, int start_col, int start_row, int goal_col, int goal_row, const BlockedCheck& is_blocked);
