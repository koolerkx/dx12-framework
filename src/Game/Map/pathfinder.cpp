#include "pathfinder.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <vector>

#include "Map/nav_grid.h"

using Math::Vector2;

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

constexpr Direction DIRECTIONS[] = {
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

PathResult Pathfinder::FindPath(const NavGrid& grid, Vector2 start_world, Vector2 goal_world, float agent_radius) {
  int grid_w = grid.GetWidth();
  int grid_h = grid.GetHeight();
  float cell_size = grid.GetCellSize();
  float origin_x = grid.GetOriginX();
  float origin_z = grid.GetOriginZ();

  int start_col, start_row, goal_col, goal_row;

  if (!grid.WorldToCell(start_world.x, start_world.y, start_col, start_row) ||
      !grid.WorldToCell(goal_world.x, goal_world.y, goal_col, goal_row)) {
    return {};
  }

  float half = cell_size * 0.5f + agent_radius;

  auto is_blocked = [&](int col, int row) -> bool {
    if (col < 0 || col >= grid_w || row < 0 || row >= grid_h) return true;
    if (agent_radius <= 0.0f) return grid.IsBlocked(col, row);
    float cx = origin_x + (col + 0.5f) * cell_size;
    float cz = origin_z + (row + 0.5f) * cell_size;
    return grid.IsAreaBlocked(cx - half, cz - half, cx + half, cz + half);
  };

  if (is_blocked(goal_col, goal_row)) {
    return {};
  }

  int total_nodes = grid_w * grid_h;
  int start_index = start_row * grid_w + start_col;
  int goal_index = goal_row * grid_w + goal_col;

  if (start_index == goal_index) {
    auto wp = grid.CellToWorld(start_col, start_row);
    return {true, {wp}};
  }

  std::vector<float> g_score(total_nodes, (std::numeric_limits<float>::max)());
  std::vector<bool> closed(total_nodes, false);
  std::vector<int> parent(total_nodes, -1);

  std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;

  g_score[start_index] = 0.0f;
  float h = OctileDistance(std::abs(goal_col - start_col), std::abs(goal_row - start_row));
  open.push({h, start_index});

  while (!open.empty()) {
    auto current = open.top();
    open.pop();

    if (current.index == goal_index) {
      std::vector<Vector2> waypoints;
      int idx = goal_index;
      while (idx != -1) {
        int c = idx % grid_w;
        int r = idx / grid_w;
        waypoints.push_back(grid.CellToWorld(c, r));
        idx = parent[idx];
      }
      std::reverse(waypoints.begin(), waypoints.end());
      return {true, std::move(waypoints)};
    }

    if (closed[current.index]) continue;
    closed[current.index] = true;

    int cur_col = current.index % grid_w;
    int cur_row = current.index / grid_w;

    for (const auto& dir : DIRECTIONS) {
      int nc = cur_col + dir.dc;
      int nr = cur_row + dir.dr;

      if (is_blocked(nc, nr)) continue;

      if (dir.dc != 0 && dir.dr != 0) {
        if (is_blocked(cur_col + dir.dc, cur_row) ||
            is_blocked(cur_col, cur_row + dir.dr)) {
          continue;
        }
      }

      int neighbor_index = nr * grid_w + nc;
      if (closed[neighbor_index]) continue;

      float tentative_g = g_score[current.index] + dir.cost;
      if (tentative_g < g_score[neighbor_index]) {
        g_score[neighbor_index] = tentative_g;
        parent[neighbor_index] = current.index;
        float nh = OctileDistance(std::abs(goal_col - nc), std::abs(goal_row - nr));
        open.push({tentative_g + nh, neighbor_index});
      }
    }
  }

  return {};
}
