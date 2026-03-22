#include "debug_drawer.h"

#include <cassert>
#include <cmath>

DebugDrawer::DebugDrawer(IDebugDrawService* service) : debug_draw_service_(service) {
}

Vector4 DebugDrawer::ApplyOpacity(const Vector4& color) const {
  return {color.x, color.y, color.z, color.w * global_opacity_};
}

void DebugDrawer::DrawLine(const Vector3& start, const Vector3& end, const Vector4& color) {
  debug_draw_service_->AddDebugLine(start, end, ApplyOpacity(color));
}

void DebugDrawer::DrawCircle(const Vector3& center, float radius, const Vector4& color, const CircleProps& props) {
  assert(props.segments > 0);
  assert(props.normal.LengthSquared() > 0.0001f);

  Vector4 final_color = ApplyOpacity(color);
  float angle_step = Math::TwoPi / props.segments;

  // Build orthonormal basis from normal
  Vector3 n = props.normal.Normalized();
  Vector3 u = (std::abs(n.y) < 0.999f) ? Vector3::Up.Cross(n).Normalized() : Vector3::Right.Cross(n).Normalized();
  Vector3 v = n.Cross(u);

  for (int i = 0; i < props.segments; ++i) {
    float a1 = i * angle_step;
    float a2 = (i + 1) * angle_step;
    Vector3 p1 = center + (u * Math::Cos(a1) + v * Math::Sin(a1)) * radius;
    Vector3 p2 = center + (u * Math::Cos(a2) + v * Math::Sin(a2)) * radius;
    debug_draw_service_->AddDebugLine(p1, p2, final_color);
  }
}

void DebugDrawer::DrawRect(const Vector3& min, const Vector3& max, const Vector4& color) {
  Vector4 final_color = ApplyOpacity(color);
  float y = (min.y + max.y) * 0.5f;

  Vector3 corners[4] = {
    {min.x, y, min.z},
    {max.x, y, min.z},
    {max.x, y, max.z},
    {min.x, y, max.z},
  };

  debug_draw_service_->AddDebugLine(corners[0], corners[1], final_color);
  debug_draw_service_->AddDebugLine(corners[1], corners[2], final_color);
  debug_draw_service_->AddDebugLine(corners[2], corners[3], final_color);
  debug_draw_service_->AddDebugLine(corners[3], corners[0], final_color);
}

void DebugDrawer::DrawWireCube(const Vector3& min, const Vector3& max, const Vector4& color) {
  Vector4 final_color = ApplyOpacity(color);

  Vector3 corners[8] = {
    {min.x, min.y, min.z},
    {max.x, min.y, min.z},
    {max.x, min.y, max.z},
    {min.x, min.y, max.z},
    {min.x, max.y, min.z},
    {max.x, max.y, min.z},
    {max.x, max.y, max.z},
    {min.x, max.y, max.z},
  };

  // Bottom face
  debug_draw_service_->AddDebugLine(corners[0], corners[1], final_color);
  debug_draw_service_->AddDebugLine(corners[1], corners[2], final_color);
  debug_draw_service_->AddDebugLine(corners[2], corners[3], final_color);
  debug_draw_service_->AddDebugLine(corners[3], corners[0], final_color);

  // Top face
  debug_draw_service_->AddDebugLine(corners[4], corners[5], final_color);
  debug_draw_service_->AddDebugLine(corners[5], corners[6], final_color);
  debug_draw_service_->AddDebugLine(corners[6], corners[7], final_color);
  debug_draw_service_->AddDebugLine(corners[7], corners[4], final_color);

  // Vertical edges
  debug_draw_service_->AddDebugLine(corners[0], corners[4], final_color);
  debug_draw_service_->AddDebugLine(corners[1], corners[5], final_color);
  debug_draw_service_->AddDebugLine(corners[2], corners[6], final_color);
  debug_draw_service_->AddDebugLine(corners[3], corners[7], final_color);
}

void DebugDrawer::DrawWireSphere(const Vector3& center, float radius, const Vector4& color, int segments) {
  assert(segments > 0);
  assert(segments % 2 == 0);

  Vector4 final_color = ApplyOpacity(color);
  float angle_step = Math::TwoPi / segments;

  for (int i = 0; i < segments; ++i) {
    float angle1 = i * angle_step;
    float angle2 = (i + 1) * angle_step;

    // XY plane circle
    {
      Vector3 p1(center.x + radius * Math::Cos(angle1), center.y + radius * Math::Sin(angle1), center.z);
      Vector3 p2(center.x + radius * Math::Cos(angle2), center.y + radius * Math::Sin(angle2), center.z);
      debug_draw_service_->AddDebugLine(p1, p2, final_color);
    }

    // XZ plane circle
    {
      Vector3 p1(center.x + radius * Math::Cos(angle1), center.y, center.z + radius * Math::Sin(angle1));
      Vector3 p2(center.x + radius * Math::Cos(angle2), center.y, center.z + radius * Math::Sin(angle2));
      debug_draw_service_->AddDebugLine(p1, p2, final_color);
    }

    // YZ plane circle
    {
      Vector3 p1(center.x, center.y + radius * Math::Cos(angle1), center.z + radius * Math::Sin(angle1));
      Vector3 p2(center.x, center.y + radius * Math::Cos(angle2), center.z + radius * Math::Sin(angle2));
      debug_draw_service_->AddDebugLine(p1, p2, final_color);
    }
  }
}

void DebugDrawer::DrawGrid(const GridConfig& config) {
  float half_size = config.size * 0.5f;

  int num_cells = static_cast<int>(config.size / config.cell_size);
  int num_lines = num_cells + 1;

  for (int i = 0; i < num_lines; ++i) {
    float x = -half_size + (i * config.cell_size);
    Vector3 start(x, config.y_level, -half_size);
    Vector3 end(x, config.y_level, half_size);

    Vector4 line_color = (std::abs(x) < 0.001f) ? config.axis_color : config.color;
    debug_draw_service_->AddDebugLine(start, end, ApplyOpacity(line_color));
  }

  for (int i = 0; i < num_lines; ++i) {
    float z = -half_size + (i * config.cell_size);
    Vector3 start(-half_size, config.y_level, z);
    Vector3 end(half_size, config.y_level, z);

    Vector4 line_color = (std::abs(z) < 0.001f) ? config.axis_color : config.color;
    debug_draw_service_->AddDebugLine(start, end, ApplyOpacity(line_color));
  }
}

void DebugDrawer::DrawAxisGizmo(const AxisGizmoConfig& config) {
  Vector3 origin = config.position;

  Vector3 x_end(origin.x + config.length, origin.y, origin.z);
  debug_draw_service_->AddDebugLine(origin, x_end, ApplyOpacity(colors::Blue));

  Vector3 y_end(origin.x, origin.y + config.length, origin.z);
  debug_draw_service_->AddDebugLine(origin, y_end, ApplyOpacity(colors::Red));

  Vector3 z_end(origin.x, origin.y, origin.z + config.length);
  debug_draw_service_->AddDebugLine(origin, z_end, ApplyOpacity(colors::Lime));
}
