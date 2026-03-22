#pragma once
#include <algorithm>

#include "Framework/Core/color.h"
#include "Framework/Math/Math.h"
#include "Framework/Render/debug_draw_service.h"

using Math::Vector3;
using Math::Vector4;

class DebugDrawer {
 public:
  explicit DebugDrawer(IDebugDrawService* service);

  void SetEnabled(bool enabled) {
    enabled_ = enabled;
  }
  bool IsEnabled() const {
    return enabled_;
  }

  void SetGlobalOpacity(float opacity) {
    global_opacity_ = std::clamp(opacity, 0.0f, 1.0f);
  }
  float GetGlobalOpacity() const {
    return global_opacity_;
  }

  // Primitives
  void DrawLine(const Vector3& start, const Vector3& end, const Vector4& color);

  struct CircleProps {
    int segments = 32;
    Vector3 normal = Vector3::Up;

    CircleProps(int segments = 32, Vector3 normal = Vector3::Up) : segments(segments), normal(normal) {};
  };
  void DrawCircle(const Vector3& center, float radius, const Vector4& color, const CircleProps& props = {});

  void DrawRect(const Vector3& min, const Vector3& max, const Vector4& color);
  void DrawWireCube(const Vector3& min, const Vector3& max, const Vector4& color);
  void DrawWireSphere(const Vector3& center, float radius, const Vector4& color = colors::Cyan, int segments = 16);

  // Grid drawing
  struct GridConfig {
    float size;
    float cell_size;
    float y_level;
    Vector4 color;
    Vector4 axis_color;

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

 private:
  Vector4 ApplyOpacity(const Vector4& color) const;

  IDebugDrawService* debug_draw_service_;
  bool enabled_ = true;
  float global_opacity_ = 1.0f;
};
