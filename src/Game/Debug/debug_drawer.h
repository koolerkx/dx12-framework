/**
 * @file debug_drawer.h
 *
 * @code {.cpp}
 * // Draw grid
 * DebugDrawer::GridConfig grid_config;
 * grid_config.size = 20.0f;
 * grid_config.cell_size = 1.0f;
 * grid_config.y_level = 0.0f;
 * grid_config.color = {0.5f, 0.5f, 0.5f, 1.0f};
 * debug_drawer->DrawGrid(grid_config);
 *
 * // Draw axis gizmo at world origin
 * DebugDrawer::AxisGizmoConfig axis_config;
 * axis_config.position = {0.0f, 0.0f, 0.0f};
 * axis_config.length = 2.0f;
 * debug_drawer->DrawAxisGizmo(axis_config);
 *
 * // Optional: Draw bounding box around cube
 * // debug_drawer->DrawBox({0, 0, 0}, {1, 1, 1}, {1, 1, 0, 1});
 * @endcode
 */
#pragma once
#include "Framework/Core/color.h"
#include "Framework/Math/Math.h"

using Math::Vector3;
using Math::Vector4;

class Graphic;

class DebugDrawer {
 public:
  explicit DebugDrawer(Graphic* graphic);

  // Grid drawing
  struct GridConfig {
    float size;       // Total grid size
    float cell_size;  // Size of each cell
    float y_level;    // Height level
    Vector4 color;
    Vector4 axis_color;  // Color for X/Z axes

    GridConfig() : size(20.0f), cell_size(1.0f), y_level(0.0f), color(colors::Gray), axis_color(colors::Silver) {
    }
  };

  void DrawGrid(const GridConfig& config = GridConfig());

  // Axis gizmo drawing
  struct AxisGizmoConfig {
    Vector3 position;
    float length;
    float thickness;

    AxisGizmoConfig() : position(Vector3::Zero), length(2.0f), thickness(2.0f) {
    }
  };

  void DrawAxisGizmo(const AxisGizmoConfig& config = AxisGizmoConfig());

  // Box drawing (useful for bounds visualization)
  void DrawBox(const Vector3& center, const Vector3& extents, const Vector4& color = colors::Yellow);

  // Sphere drawing (approximated with lines)
  void DrawSphere(const Vector3& center, float radius, const Vector4& color = colors::Cyan, int segments = 16);

 private:
  Graphic* graphic_;
};
