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
#include <DirectXMath.h>

// Forward declaration
class Graphic;

class DebugDrawer {
 public:
  explicit DebugDrawer(Graphic* graphic);

  // Grid drawing
  struct GridConfig {
    float size;       // Total grid size
    float cell_size;  // Size of each cell
    float y_level;    // Height level
    DirectX::XMFLOAT4 color;
    DirectX::XMFLOAT4 axis_color;  // Color for X/Z axes

    GridConfig() : size(20.0f), cell_size(1.0f), y_level(0.0f), color(0.5f, 0.5f, 0.5f, 1.0f), axis_color(0.7f, 0.7f, 0.7f, 1.0f) {
    }
  };

  void DrawGrid(const GridConfig& config = GridConfig());

  // Axis gizmo drawing
  struct AxisGizmoConfig {
    DirectX::XMFLOAT3 position;
    float length;
    float thickness;  // For future thick line support

    AxisGizmoConfig() : position(0.0f, 0.0f, 0.0f), length(2.0f), thickness(2.0f) {
    }
  };

  void DrawAxisGizmo(const AxisGizmoConfig& config = AxisGizmoConfig());

  // Box drawing (useful for bounds visualization)
  void DrawBox(const DirectX::XMFLOAT3& center, const DirectX::XMFLOAT3& extents, const DirectX::XMFLOAT4& color = {1, 1, 0, 1});

  // Sphere drawing (approximated with lines)
  void DrawSphere(const DirectX::XMFLOAT3& center, float radius, const DirectX::XMFLOAT4& color = {0, 1, 1, 1}, int segments = 16);

 private:
  Graphic* graphic_;
};
