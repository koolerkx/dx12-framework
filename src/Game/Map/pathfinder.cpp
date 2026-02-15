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

Vector2 NodeToWorld(int col, int row, float origin_x, float origin_z, float node_size) {
  return {origin_x + (col + 0.5f) * node_size, origin_z + (row + 0.5f) * node_size};
}

bool IsNodeBlocked(const NavGrid& grid, int col, int row, float origin_x, float origin_z, float node_size) {
  float cx = origin_x + (col + 0.5f) * node_size;
  float cz = origin_z + (row + 0.5f) * node_size;
  float half = node_size * 0.5f;
  return grid.IsAreaBlocked(cx - half, cz - half, cx + half, cz + half);
}

}  // namespace

PathResult Pathfinder::FindPath(const NavGrid& grid, Vector2 start_world, Vector2 goal_world, float node_size) {
  float origin_x = grid.GetOriginX();
  float origin_z = grid.GetOriginZ();
  float world_w = grid.GetWorldWidth();
  float world_h = grid.GetWorldHeight();

  int grid_w = static_cast<int>(std::ceil(world_w / node_size));
  int grid_h = static_cast<int>(std::ceil(world_h / node_size));

  auto world_to_node = [&](float wx, float wz, int& out_col, int& out_row) -> bool {
    out_col = static_cast<int>(std::floor((wx - origin_x) / node_size));
    out_row = static_cast<int>(std::floor((wz - origin_z) / node_size));
    return out_col >= 0 && out_col < grid_w && out_row >= 0 && out_row < grid_h;
  };

  int start_col, start_row, goal_col, goal_row;

  if (!world_to_node(start_world.x, start_world.y, start_col, start_row) ||
      !world_to_node(goal_world.x, goal_world.y, goal_col, goal_row)) {
    return {};
  }

  if (IsNodeBlocked(grid, start_col, start_row, origin_x, origin_z, node_size) ||
      IsNodeBlocked(grid, goal_col, goal_row, origin_x, origin_z, node_size)) {
    return {};
  }

  int total_nodes = grid_w * grid_h;
  int start_index = start_row * grid_w + start_col;
  int goal_index = goal_row * grid_w + goal_col;

  if (start_index == goal_index) {
    return {true, {NodeToWorld(start_col, start_row, origin_x, origin_z, node_size)}};
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
        waypoints.push_back(NodeToWorld(c, r, origin_x, origin_z, node_size));
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

      if (nc < 0 || nc >= grid_w || nr < 0 || nr >= grid_h) continue;
      if (IsNodeBlocked(grid, nc, nr, origin_x, origin_z, node_size)) continue;

      bool is_diagonal = (dir.dc != 0 && dir.dr != 0);
      if (is_diagonal) {
        if (IsNodeBlocked(grid, cur_col + dir.dc, cur_row, origin_x, origin_z, node_size) ||
            IsNodeBlocked(grid, cur_col, cur_row + dir.dr, origin_x, origin_z, node_size)) {
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
