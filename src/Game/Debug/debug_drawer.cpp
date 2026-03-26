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

  Vector3 n = props.normal.Normalized();
  Vector3 u = (std::abs(n.y) < 0.999f) ? Vector3::Up.Cross(n).Normalized() : Vector3::Right.Cross(n).Normalized();
  Vector3 v = n.Cross(u);

  debug_draw_service_->AddDebugLine(LineSetId::Circle32, center, u * radius, v * radius, {}, ApplyOpacity(color));
}

void DebugDrawer::DrawRect(const Vector3& min, const Vector3& max, const Vector4& color) {
  Vector3 center = (min + max) * 0.5f;
  Vector3 half_extent = (max - min) * 0.5f;
  debug_draw_service_->AddDebugLine(LineSetId::UnitRect, center, {half_extent.x, 0, 0}, {}, {0, 0, half_extent.z}, ApplyOpacity(color));
}

void DebugDrawer::DrawWireCube(const Vector3& min, const Vector3& max, const Vector4& color) {
  Vector3 center = (min + max) * 0.5f;
  Vector3 half_extent = (max - min) * 0.5f;
  debug_draw_service_->AddDebugLine(
    LineSetId::UnitCube, center, {half_extent.x, 0, 0}, {0, half_extent.y, 0}, {0, 0, half_extent.z}, ApplyOpacity(color));
}

void DebugDrawer::DrawWireSphere(const Vector3& center, float radius, const Vector4& color, int segments) {
  LineSetId id = (segments <= 8) ? LineSetId::Sphere8 : (segments <= 16) ? LineSetId::Sphere16 : LineSetId::Sphere32;
  Vector4 c = ApplyOpacity(color);
  debug_draw_service_->AddDebugLine(id, center, {radius, 0, 0}, {0, radius, 0}, {0, 0, radius}, c);
}

void DebugDrawer::DrawGrid(const GridConfig& config) {
  float half_size = config.size * 0.5f;
  int num_cells = static_cast<int>(config.size / config.cell_size);
  int num_lines = num_cells + 1;

  for (int i = 0; i < num_lines; ++i) {
    float x = -half_size + (i * config.cell_size);
    Vector4 line_color = ApplyOpacity((std::abs(x) < 0.001f) ? config.axis_color : config.color);
    debug_draw_service_->AddDebugLine({x, config.y_level, -half_size}, {x, config.y_level, half_size}, line_color);
  }

  for (int i = 0; i < num_lines; ++i) {
    float z = -half_size + (i * config.cell_size);
    Vector4 line_color = ApplyOpacity((std::abs(z) < 0.001f) ? config.axis_color : config.color);
    debug_draw_service_->AddDebugLine({-half_size, config.y_level, z}, {half_size, config.y_level, z}, line_color);
  }
}

void DebugDrawer::DrawAxisGizmo(const AxisGizmoConfig& config) {
  Vector3 origin = config.position;
  debug_draw_service_->AddDebugLine(origin, {origin.x + config.length, origin.y, origin.z}, ApplyOpacity(colors::Blue));
  debug_draw_service_->AddDebugLine(origin, {origin.x, origin.y + config.length, origin.z}, ApplyOpacity(colors::Red));
  debug_draw_service_->AddDebugLine(origin, {origin.x, origin.y, origin.z + config.length}, ApplyOpacity(colors::Lime));
}
