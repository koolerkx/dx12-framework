#pragma once

#include <cstdint>
#include <vector>

#include "Component/component.h"
#include "Framework/Math/Math.h"

struct MapData;

struct PathResult {
  bool found = false;
  std::vector<Math::Vector2> waypoints;
};

class NavGridComponent : public Component<NavGridComponent> {
 public:
  struct Props {
    float cell_size = 0.25f;
    float block_threshold = 0.5f;
    float node_size = 0.25f;
    float waypoint_reach_threshold = 0.15f;
    bool show_debug_grid = false;
  };

  NavGridComponent(GameObject* owner, const Props& props);

  void Build(const MapData& map_data, const std::vector<Math::AABB>& obstacles);
  PathResult FindPath(Math::Vector2 start, Math::Vector2 goal, float agent_radius) const;

  int GetWidth() const { return width_; }
  int GetHeight() const { return height_; }
  float GetWaypointReachThreshold() const { return waypoint_reach_threshold_; }

  void OnDebugDraw(DebugDrawer& drawer) override;

 private:
  int CellIndex(int col, int row) const { return row * width_ + col; }

  bool IsBlocked(int col, int row) const;
  bool IsInBounds(int col, int row) const;
  bool IsAreaBlocked(float min_x, float min_z, float max_x, float max_z) const;

  Math::Vector2 NodeToWorld(int col, int row) const;
  bool WorldToNode(float wx, float wz, int& out_col, int& out_row) const;

  float cell_size_ = 0.25f;
  float block_threshold_ = 0.5f;
  float node_size_ = 0.25f;
  float waypoint_reach_threshold_ = 0.15f;
  bool show_debug_grid_ = false;

  int width_ = 0;
  int height_ = 0;
  float origin_x_ = 0.0f;
  float origin_z_ = 0.0f;
  std::vector<uint8_t> cells_;
};
