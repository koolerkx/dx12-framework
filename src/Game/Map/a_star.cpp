#include "a_star.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <span>

namespace {

constexpr float SQRT2 = 1.41421356f;

struct Node {
  float f_score;
  int index;

  bool operator>(const Node& other) const {
    return f_score > other.f_score;
  }
};

float OctileDistance(int dx, int dz) {
  int d_min = (std::min)(dx, dz);
  int d_max = (std::max)(dx, dz);
  return static_cast<float>(d_max - d_min) + SQRT2 * static_cast<float>(d_min);
}

struct Direction {
  int dc, dr;
  float cost;
};

constexpr Direction ALL_DIRECTIONS[] = {
  {1, 0, 1.0f},
  {-1, 0, 1.0f},
  {0, 1, 1.0f},
  {0, -1, 1.0f},
  {1, 1, SQRT2},
  {1, -1, SQRT2},
  {-1, 1, SQRT2},
  {-1, -1, SQRT2},
};

}  // namespace

AStarResult FindPathAStar(
  int grid_width, int grid_height, int start_col, int start_row, int goal_col, int goal_row, const BlockedCheck& is_blocked) {
  if (start_col < 0 || start_col >= grid_width || start_row < 0 || start_row >= grid_height) return {};
  if (goal_col < 0 || goal_col >= grid_width || goal_row < 0 || goal_row >= grid_height) return {};
  if (is_blocked(start_col, start_row) || is_blocked(goal_col, goal_row)) return {};

  int total_nodes = grid_width * grid_height;
  int start_index = start_row * grid_width + start_col;
  int goal_index = goal_row * grid_width + goal_col;

  if (start_index == goal_index) {
    return {true, {start_index}};
  }

  std::vector<float> g_score(total_nodes, (std::numeric_limits<float>::max)());
  std::vector<bool> closed(total_nodes, false);
  std::vector<int> parent(total_nodes, -1);

  std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;

  auto directions = std::span<const Direction>(ALL_DIRECTIONS);

  auto heuristic = [&](int dc, int dr) -> float { return OctileDistance(std::abs(dc), std::abs(dr)); };

  g_score[start_index] = 0.0f;
  open.push({heuristic(goal_col - start_col, goal_row - start_row), start_index});

  while (!open.empty()) {
    auto current = open.top();
    open.pop();

    if (current.index == goal_index) {
      std::vector<int> path;
      int idx = goal_index;
      while (idx != -1) {
        path.push_back(idx);
        idx = parent[idx];
      }
      std::reverse(path.begin(), path.end());
      return {true, std::move(path)};
    }

    if (closed[current.index]) continue;
    closed[current.index] = true;

    int cur_col = current.index % grid_width;
    int cur_row = current.index / grid_width;

    for (const auto& dir : directions) {
      int nc = cur_col + dir.dc;
      int nr = cur_row + dir.dr;

      if (nc < 0 || nc >= grid_width || nr < 0 || nr >= grid_height) continue;
      if (is_blocked(nc, nr)) continue;

      bool is_diagonal = (dir.dc != 0 && dir.dr != 0);
      if (is_diagonal) {
        if (is_blocked(cur_col + dir.dc, cur_row) || is_blocked(cur_col, cur_row + dir.dr)) {
          continue;
        }
      }

      int neighbor_index = nr * grid_width + nc;
      if (closed[neighbor_index]) continue;

      float tentative_g = g_score[current.index] + dir.cost;
      if (tentative_g < g_score[neighbor_index]) {
        g_score[neighbor_index] = tentative_g;
        parent[neighbor_index] = current.index;
        float nh = heuristic(goal_col - nc, goal_row - nr);
        open.push({tentative_g + nh, neighbor_index});
      }
    }
  }

  return {};
}
