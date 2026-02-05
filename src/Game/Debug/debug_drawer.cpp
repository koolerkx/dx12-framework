#include "debug_drawer.h"

#include <cmath>

#include "Graphic/graphic.h"

DebugDrawer::DebugDrawer(Graphic* graphic) : graphic_(graphic) {
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
    graphic_->AddDebugLine(start, end, line_color);
  }

  for (int i = 0; i < num_lines; ++i) {
    float z = -half_size + (i * config.cell_size);
    Vector3 start(-half_size, config.y_level, z);
    Vector3 end(half_size, config.y_level, z);

    Vector4 line_color = (std::abs(z) < 0.001f) ? config.axis_color : config.color;
    graphic_->AddDebugLine(start, end, line_color);
  }
}

void DebugDrawer::DrawAxisGizmo(const AxisGizmoConfig& config) {
  Vector3 origin = config.position;

  Vector3 x_end(origin.x + config.length, origin.y, origin.z);
  graphic_->AddDebugLine(origin, x_end, Vector4(0.0f, 0.0f, 1.0f, 1.0f));

  Vector3 y_end(origin.x, origin.y + config.length, origin.z);
  graphic_->AddDebugLine(origin, y_end, Vector4(1.0f, 0.0f, 0.0f, 1.0f));

  Vector3 z_end(origin.x, origin.y, origin.z + config.length);
  graphic_->AddDebugLine(origin, z_end, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
}

void DebugDrawer::DrawBox(const Vector3& center, const Vector3& extents, const Vector4& color) {
  Vector3 corners[8] = {
    {center.x - extents.x, center.y - extents.y, center.z - extents.z},
    {center.x + extents.x, center.y - extents.y, center.z - extents.z},
    {center.x + extents.x, center.y - extents.y, center.z + extents.z},
    {center.x - extents.x, center.y - extents.y, center.z + extents.z},
    {center.x - extents.x, center.y + extents.y, center.z - extents.z},
    {center.x + extents.x, center.y + extents.y, center.z - extents.z},
    {center.x + extents.x, center.y + extents.y, center.z + extents.z},
    {center.x - extents.x, center.y + extents.y, center.z + extents.z},
  };

  // Bottom face
  graphic_->AddDebugLine(corners[0], corners[1], color);
  graphic_->AddDebugLine(corners[1], corners[2], color);
  graphic_->AddDebugLine(corners[2], corners[3], color);
  graphic_->AddDebugLine(corners[3], corners[0], color);

  // Top face
  graphic_->AddDebugLine(corners[4], corners[5], color);
  graphic_->AddDebugLine(corners[5], corners[6], color);
  graphic_->AddDebugLine(corners[6], corners[7], color);
  graphic_->AddDebugLine(corners[7], corners[4], color);

  // Vertical edges
  graphic_->AddDebugLine(corners[0], corners[4], color);
  graphic_->AddDebugLine(corners[1], corners[5], color);
  graphic_->AddDebugLine(corners[2], corners[6], color);
  graphic_->AddDebugLine(corners[3], corners[7], color);
}

/**
 * @brief Draws a sphere
 *
 * @param segments defned how many segments to draw, total line drawn will be segments * 3
 */
void DebugDrawer::DrawSphere(const Vector3& center, float radius, const Vector4& color, int segments) {
  assert(segments > 0);
  assert(segments % 2 == 0);

  float angle_step = Math::TwoPi / segments;

  // Draw circles in XY, XZ, and YZ planes
  for (int i = 0; i < segments; ++i) {
    float angle1 = i * angle_step;
    float angle2 = (i + 1) * angle_step;

    // XY plane circle
    {
      Vector3 p1(center.x + radius * Math::Cos(angle1), center.y + radius * Math::Sin(angle1), center.z);
      Vector3 p2(center.x + radius * Math::Cos(angle2), center.y + radius * Math::Sin(angle2), center.z);
      graphic_->AddDebugLine(p1, p2, color);
    }

    // XZ plane circle
    {
      Vector3 p1(center.x + radius * Math::Cos(angle1), center.y, center.z + radius * Math::Sin(angle1));
      Vector3 p2(center.x + radius * Math::Cos(angle2), center.y, center.z + radius * Math::Sin(angle2));
      graphic_->AddDebugLine(p1, p2, color);
    }

    // YZ plane circle
    {
      Vector3 p1(center.x, center.y + radius * Math::Cos(angle1), center.z + radius * Math::Sin(angle1));
      Vector3 p2(center.x, center.y + radius * Math::Cos(angle2), center.z + radius * Math::Sin(angle2));
      graphic_->AddDebugLine(p1, p2, color);
    }
  }
}
