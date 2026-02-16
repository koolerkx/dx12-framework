#include "nav_grid_component.h"

#include <chrono>
#include <cmath>

#include "Debug/debug_drawer.h"
#include "Framework/Logging/logger.h"
#include "Map/a_star.h"
#include "Map/map_data.h"

using Math::Vector2;
using Math::Vector4;

NavGridComponent::NavGridComponent(GameObject* owner, const Props& props)
    : Component(owner),
      cell_size_(props.cell_size),
      block_threshold_(props.block_threshold),
      node_size_(props.node_size),
      waypoint_reach_threshold_(props.waypoint_reach_threshold),
      show_debug_grid_(props.show_debug_grid) {
}

void NavGridComponent::Build(const MapData& map_data, const std::vector<Math::AABB>& obstacles) {
  auto start_time = std::chrono::steady_clock::now();

  auto bounds = ComputeGroundBounds(map_data);
  if (bounds.min_x > bounds.max_x) {
    Logger::LogFormat(LogLevel::Warn, LogCategory::Game, Logger::Here(), "[NavGrid] No ground layer items found, skipping build");
    return;
  }
  origin_x_ = bounds.min_x;
  origin_z_ = bounds.min_z;
  float extent_x = bounds.max_x - origin_x_;
  float extent_z = bounds.max_z - origin_z_;

  width_ = static_cast<int>(std::ceil(extent_x / cell_size_));
  height_ = static_cast<int>(std::ceil(extent_z / cell_size_));
  cells_.assign(width_ * height_, 0);

  float cell_area = cell_size_ * cell_size_;
  float min_overlap_area = cell_area * block_threshold_;

  for (const auto& aabb : obstacles) {
    // Shrink AABB inward by epsilon to avoid marking extra cells on exact boundary alignment
    constexpr float EPSILON = 1e-4f;
    int col_min = static_cast<int>(std::floor((aabb.min.x - origin_x_ + EPSILON) / cell_size_));
    int col_max = static_cast<int>(std::floor((aabb.max.x - origin_x_ - EPSILON) / cell_size_));
    int row_min = static_cast<int>(std::floor((aabb.min.z - origin_z_ + EPSILON) / cell_size_));
    int row_max = static_cast<int>(std::floor((aabb.max.z - origin_z_ - EPSILON) / cell_size_));

    col_min = (std::max)(col_min, 0);
    col_max = (std::min)(col_max, width_ - 1);
    row_min = (std::max)(row_min, 0);
    row_max = (std::min)(row_max, height_ - 1);

    for (int r = row_min; r <= row_max; ++r) {
      for (int c = col_min; c <= col_max; ++c) {
        float cell_x0 = origin_x_ + c * cell_size_;
        float cell_z0 = origin_z_ + r * cell_size_;
        float cell_x1 = cell_x0 + cell_size_;
        float cell_z1 = cell_z0 + cell_size_;

        float ox = (std::min)(cell_x1, aabb.max.x) - (std::max)(cell_x0, aabb.min.x);
        float oz = (std::min)(cell_z1, aabb.max.z) - (std::max)(cell_z0, aabb.min.z);

        if (ox > 0.0f && oz > 0.0f && ox * oz >= min_overlap_area) {
          cells_[CellIndex(c, r)] = 1;
        }
      }
    }
  }

  auto elapsed_ms = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - start_time).count();
  Logger::LogFormat(LogLevel::Info,
    LogCategory::Game,
    Logger::Here(),
    "[NavGrid] Built {}x{} grid (cell_size={}, obstacles={}) in {:.2f}ms",
    width_,
    height_,
    cell_size_,
    obstacles.size(),
    elapsed_ms);
}

PathResult NavGridComponent::FindPath(Vector2 start, Vector2 goal, float agent_radius) const {
  float world_w = width_ * cell_size_;
  float world_h = height_ * cell_size_;

  int node_grid_w = static_cast<int>(std::ceil(world_w / node_size_));
  int node_grid_h = static_cast<int>(std::ceil(world_h / node_size_));

  auto world_to_node = [&](float wx, float wz, int& out_col, int& out_row) -> bool {
    out_col = static_cast<int>(std::floor((wx - origin_x_) / node_size_));
    out_row = static_cast<int>(std::floor((wz - origin_z_) / node_size_));
    return out_col >= 0 && out_col < node_grid_w && out_row >= 0 && out_row < node_grid_h;
  };

  int start_col, start_row, goal_col, goal_row;
  if (!world_to_node(start.x, start.y, start_col, start_row) ||
      !world_to_node(goal.x, goal.y, goal_col, goal_row)) {
    return {};
  }

  constexpr float EPSILON = 1e-4f;

  auto is_node_blocked = [&](int col, int row) -> bool {
    float cx = origin_x_ + (col + 0.5f) * node_size_;
    float cz = origin_z_ + (row + 0.5f) * node_size_;
    float half = node_size_ * 0.5f + agent_radius - EPSILON;
    return IsAreaBlocked(cx - half, cz - half, cx + half, cz + half);
  };

  auto result = FindPathAStar(node_grid_w, node_grid_h, start_col, start_row, goal_col, goal_row, is_node_blocked);
  if (!result.found) return {};

  std::vector<Vector2> waypoints;
  waypoints.reserve(result.path.size());
  for (int idx : result.path) {
    int c = idx % node_grid_w;
    int r = idx / node_grid_w;
    waypoints.push_back(NodeToWorld(c, r));
  }

  return {true, std::move(waypoints)};
}

bool NavGridComponent::IsBlocked(int col, int row) const {
  if (!IsInBounds(col, row)) return true;
  return cells_[CellIndex(col, row)] != 0;
}

bool NavGridComponent::IsInBounds(int col, int row) const {
  return col >= 0 && col < width_ && row >= 0 && row < height_;
}

bool NavGridComponent::IsAreaBlocked(float min_x, float min_z, float max_x, float max_z) const {
  int col_min = static_cast<int>(std::floor((min_x - origin_x_) / cell_size_));
  int col_max = static_cast<int>(std::floor((max_x - origin_x_) / cell_size_));
  int row_min = static_cast<int>(std::floor((min_z - origin_z_) / cell_size_));
  int row_max = static_cast<int>(std::floor((max_z - origin_z_) / cell_size_));

  col_min = (std::max)(col_min, 0);
  col_max = (std::min)(col_max, width_ - 1);
  row_min = (std::max)(row_min, 0);
  row_max = (std::min)(row_max, height_ - 1);

  for (int r = row_min; r <= row_max; ++r) {
    for (int c = col_min; c <= col_max; ++c) {
      if (cells_[CellIndex(c, r)]) return true;
    }
  }
  return false;
}

Vector2 NavGridComponent::NodeToWorld(int col, int row) const {
  float x = origin_x_ + (col + 0.5f) * node_size_;
  float z = origin_z_ + (row + 0.5f) * node_size_;
  return {x, z};
}

bool NavGridComponent::WorldToNode(float wx, float wz, int& out_col, int& out_row) const {
  out_col = static_cast<int>(std::floor((wx - origin_x_) / node_size_));
  out_row = static_cast<int>(std::floor((wz - origin_z_) / node_size_));
  float world_w = width_ * cell_size_;
  float world_h = height_ * cell_size_;
  int node_grid_w = static_cast<int>(std::ceil(world_w / node_size_));
  int node_grid_h = static_cast<int>(std::ceil(world_h / node_size_));
  return out_col >= 0 && out_col < node_grid_w && out_row >= 0 && out_row < node_grid_h;
}

void NavGridComponent::OnDebugDraw(DebugDrawer& drawer) {
  if (!show_debug_grid_) return;

  const Vector4 blocked_color = {1.0f, 0.0f, 0.0f, 0.4f};
  const Vector4 free_color = {0.0f, 1.0f, 0.0f, 0.1f};

  constexpr float DEBUG_Y = 0.1f;

  for (int r = 0; r < height_; ++r) {
    for (int c = 0; c < width_; ++c) {
      float x0 = origin_x_ + c * cell_size_;
      float z0 = origin_z_ + r * cell_size_;
      float x1 = x0 + cell_size_;
      float z1 = z0 + cell_size_;

      const auto& color = cells_[CellIndex(c, r)] ? blocked_color : free_color;
      drawer.DrawRect({x0, DEBUG_Y, z0}, {x1, DEBUG_Y, z1}, color);
    }
  }
}
