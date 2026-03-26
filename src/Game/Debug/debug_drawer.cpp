#include "debug_drawer.h"

#include <cassert>
#include <cmath>

#include "Framework/Math/unit_circle_lut.h"
#include "Framework/Render/render_service.h"

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

  Vector3 n = props.normal.Normalized();
  Vector3 u = (std::abs(n.y) < 0.999f) ? Vector3::Up.Cross(n).Normalized() : Vector3::Right.Cross(n).Normalized();
  Vector3 v = n.Cross(u);

  const auto& lut = Math::GetCircleLUT<32>();
  const int seg = (std::min)(props.segments, static_cast<int>(lut.points.size()));

  auto span = debug_draw_service_->ReserveDebugLines(seg);
  for (int i = 0; i < seg; ++i) {
    int next = (i + 1) % seg;
    uint32_t idx = static_cast<uint32_t>(i) * 2;
    span[idx] = {center + (u * lut.points[i].cos + v * lut.points[i].sin) * radius, final_color};
    span[idx + 1] = {center + (u * lut.points[next].cos + v * lut.points[next].sin) * radius, final_color};
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

  auto span = debug_draw_service_->ReserveDebugLines(4);
  for (int i = 0; i < 4; ++i) {
    span[i * 2] = {corners[i], final_color};
    span[i * 2 + 1] = {corners[(i + 1) % 4], final_color};
  }
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

  auto span = debug_draw_service_->ReserveDebugLines(12);
  uint32_t idx = 0;

  // Bottom face
  for (int i = 0; i < 4; ++i) {
    span[idx++] = {corners[i], final_color};
    span[idx++] = {corners[(i + 1) % 4], final_color};
  }
  // Top face
  for (int i = 4; i < 8; ++i) {
    span[idx++] = {corners[i], final_color};
    span[idx++] = {corners[4 + (i - 4 + 1) % 4], final_color};
  }
  // Vertical edges
  for (int i = 0; i < 4; ++i) {
    span[idx++] = {corners[i], final_color};
    span[idx++] = {corners[i + 4], final_color};
  }
}

void DebugDrawer::DrawWireSphere(const Vector3& center, float radius, const Vector4& color, int segments) {
  assert(segments > 0);
  assert(segments % 2 == 0);

  Vector4 final_color = ApplyOpacity(color);
  const auto& lut = Math::GetCircleLUT<16>();
  const int seg = (std::min)(segments, static_cast<int>(lut.points.size()));

  auto span = debug_draw_service_->ReserveDebugLines(seg * 3);
  uint32_t idx = 0;

  for (int i = 0; i < seg; ++i) {
    int next = (i + 1) % seg;
    float c1 = lut.points[i].cos * radius;
    float s1 = lut.points[i].sin * radius;
    float c2 = lut.points[next].cos * radius;
    float s2 = lut.points[next].sin * radius;

    // XY plane
    span[idx++] = {{center.x + c1, center.y + s1, center.z}, final_color};
    span[idx++] = {{center.x + c2, center.y + s2, center.z}, final_color};

    // XZ plane
    span[idx++] = {{center.x + c1, center.y, center.z + s1}, final_color};
    span[idx++] = {{center.x + c2, center.y, center.z + s2}, final_color};

    // YZ plane
    span[idx++] = {{center.x, center.y + c1, center.z + s1}, final_color};
    span[idx++] = {{center.x, center.y + c2, center.z + s2}, final_color};
  }
}

void DebugDrawer::DrawGrid(const GridConfig& config) {
  float half_size = config.size * 0.5f;

  int num_cells = static_cast<int>(config.size / config.cell_size);
  int num_lines = num_cells + 1;

  auto span = debug_draw_service_->ReserveDebugLines(num_lines * 2);
  uint32_t idx = 0;

  for (int i = 0; i < num_lines; ++i) {
    float x = -half_size + (i * config.cell_size);
    Vector4 line_color = ApplyOpacity((std::abs(x) < 0.001f) ? config.axis_color : config.color);
    span[idx++] = {{x, config.y_level, -half_size}, line_color};
    span[idx++] = {{x, config.y_level, half_size}, line_color};
  }

  for (int i = 0; i < num_lines; ++i) {
    float z = -half_size + (i * config.cell_size);
    Vector4 line_color = ApplyOpacity((std::abs(z) < 0.001f) ? config.axis_color : config.color);
    span[idx++] = {{-half_size, config.y_level, z}, line_color};
    span[idx++] = {{half_size, config.y_level, z}, line_color};
  }
}

void DebugDrawer::DrawAxisGizmo(const AxisGizmoConfig& config) {
  Vector3 origin = config.position;

  auto span = debug_draw_service_->ReserveDebugLines(3);
  span[0] = {origin, ApplyOpacity(colors::Blue)};
  span[1] = {{origin.x + config.length, origin.y, origin.z}, ApplyOpacity(colors::Blue)};
  span[2] = {origin, ApplyOpacity(colors::Red)};
  span[3] = {{origin.x, origin.y + config.length, origin.z}, ApplyOpacity(colors::Red)};
  span[4] = {origin, ApplyOpacity(colors::Lime)};
  span[5] = {{origin.x, origin.y, origin.z + config.length}, ApplyOpacity(colors::Lime)};
}
