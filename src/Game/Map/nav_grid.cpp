#include "nav_grid.h"

#include <chrono>
#include <cmath>
#include <limits>

#include "Debug/debug_drawer.h"
#include "Framework/Logging/logger.h"
#include "Map/map_data.h"

using Math::Vector2;
using Math::Vector4;

namespace {

struct XZBounds {
  float min_x = (std::numeric_limits<float>::max)();
  float max_x = (std::numeric_limits<float>::lowest)();
  float min_z = (std::numeric_limits<float>::max)();
  float max_z = (std::numeric_limits<float>::lowest)();

  void Expand(float x, float z, float half_sx, float half_sz) {
    min_x = (std::min)(min_x, x - half_sx);
    max_x = (std::max)(max_x, x + half_sx);
    min_z = (std::min)(min_z, z - half_sz);
    max_z = (std::max)(max_z, z + half_sz);
  }
};

// Out ground must be 1*1 tile
XZBounds ComputeGroundBounds(const MapData& map_data) {
  XZBounds bounds;
  for (const auto& layer : map_data.layers) {
    if (layer.id != "ground") continue;
    for (const auto& item : layer.items) {
      float world_x = item.transform.x + map_data.origin_x;
      float world_z = item.transform.z + map_data.origin_z;
      float half_sx = 0.5f * item.transform.scale_x;
      float half_sz = 0.5f * item.transform.scale_z;
      bounds.Expand(world_x, world_z, half_sx, half_sz);
    }
  }
  return bounds;
}

}  // namespace

void NavGrid::Build(const MapData& map_data, const std::vector<Math::AABB>& obstacles, const NavGridConfig& config) {
  auto start_time = std::chrono::steady_clock::now();

  show_debug_grid_ = config.show_debug_grid;
  cell_size_ = config.cell_size;

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
  float min_overlap_area = cell_area * config.block_threshold;

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

    // Check overlap threshold
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

bool NavGrid::IsBlocked(int col, int row) const {
  if (!IsInBounds(col, row)) return true;
  return cells_[CellIndex(col, row)] != 0;
}

bool NavGrid::IsInBounds(int col, int row) const {
  return col >= 0 && col < width_ && row >= 0 && row < height_;
}

bool NavGrid::IsAreaBlocked(float min_x, float min_z, float max_x, float max_z) const {
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

Vector2 NavGrid::CellToWorld(int col, int row) const {
  float x = origin_x_ + (col + 0.5f) * cell_size_;
  float z = origin_z_ + (row + 0.5f) * cell_size_;
  return {x, z};
}

bool NavGrid::WorldToCell(float world_x, float world_z, int& out_col, int& out_row) const {
  out_col = static_cast<int>(std::floor((world_x - origin_x_) / cell_size_));
  out_row = static_cast<int>(std::floor((world_z - origin_z_) / cell_size_));
  return IsInBounds(out_col, out_row);
}

void NavGrid::DebugDraw(DebugDrawer& drawer, float y_level) const {
  if (!show_debug_grid_) return;

  const Vector4 blocked_color = {1.0f, 0.0f, 0.0f, 0.4f};
  const Vector4 free_color = {0.0f, 1.0f, 0.0f, 0.1f};

  for (int r = 0; r < height_; ++r) {
    for (int c = 0; c < width_; ++c) {
      float x0 = origin_x_ + c * cell_size_;
      float z0 = origin_z_ + r * cell_size_;
      float x1 = x0 + cell_size_;
      float z1 = z0 + cell_size_;

      const auto& color = cells_[CellIndex(c, r)] ? blocked_color : free_color;
      drawer.DrawRect({x0, y_level, z0}, {x1, y_level, z1}, color);
    }
  }
}
