#pragma once

#include <cstdint>
#include <vector>

#include "Framework/Math/Math.h"

struct MapData;
class DebugDrawer;

struct NavGridConfig {
  float cell_size = 1.0f;
  float block_threshold = 0.5f;
  bool show_debug_grid = false;
};

class NavGrid {
 public:
  void Build(const MapData& map_data, const std::vector<Math::AABB>& obstacles, const NavGridConfig& config = {});

  bool IsBlocked(int col, int row) const;
  bool IsInBounds(int col, int row) const;
  bool IsAreaBlocked(float min_x, float min_z, float max_x, float max_z) const;

  void BlockArea(const Math::AABB& bounds);
  void UnblockArea(const Math::AABB& bounds);

  Math::Vector2 CellToWorld(int col, int row) const;
  bool WorldToCell(float world_x, float world_z, int& out_col, int& out_row) const;

  int GetWidth() const {
    return width_;
  }
  int GetHeight() const {
    return height_;
  }
  float GetOriginX() const {
    return origin_x_;
  }
  float GetOriginZ() const {
    return origin_z_;
  }
  float GetWorldWidth() const {
    return width_ * cell_size_;
  }
  float GetWorldHeight() const {
    return height_ * cell_size_;
  }

  void DebugDraw(DebugDrawer& drawer, float y_level) const;

 private:
  int CellIndex(int col, int row) const {
    return row * width_ + col;
  }
  void SetAreaValue(const Math::AABB& bounds, uint8_t value);

  bool show_debug_grid_ = false;
  float cell_size_ = 1.0f;
  int width_ = 0;
  int height_ = 0;
  float origin_x_ = 0.0f;
  float origin_z_ = 0.0f;
  std::vector<uint8_t> cells_;
};
