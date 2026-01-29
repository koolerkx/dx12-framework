#include "debug_drawer.h"

#include <cmath>

#include "Graphic/graphic.h"

DebugDrawer::DebugDrawer(Graphic* graphic) : graphic_(graphic) {
}

void DebugDrawer::DrawGrid(const GridConfig& config) {
  using namespace DirectX;

  // Calculate range: grid_size is total size, so half on each side
  float half_size = config.size * 0.5f;

  // Calculate number of lines based on cell size
  int num_cells = static_cast<int>(config.size / config.cell_size);
  int num_lines = num_cells + 1;  // N cells need N+1 lines

  // Lines parallel to Z-axis (along X direction)
  for (int i = 0; i < num_lines; ++i) {
    float x = -half_size + (i * config.cell_size);
    XMFLOAT3 start(x, config.y_level, -half_size);
    XMFLOAT3 end(x, config.y_level, half_size);

    // Use brighter color for X axis (x=0)
    XMFLOAT4 line_color = (std::abs(x) < 0.001f) ? config.axis_color : config.color;
    graphic_->AddDebugLine(start, end, line_color);
  }

  // Lines parallel to X-axis (along Z direction)
  for (int i = 0; i < num_lines; ++i) {
    float z = -half_size + (i * config.cell_size);
    XMFLOAT3 start(-half_size, config.y_level, z);
    XMFLOAT3 end(half_size, config.y_level, z);

    // Use brighter color for Z axis (z=0)
    XMFLOAT4 line_color = (std::abs(z) < 0.001f) ? config.axis_color : config.color;
    graphic_->AddDebugLine(start, end, line_color);
  }
}

void DebugDrawer::DrawAxisGizmo(const AxisGizmoConfig& config) {
  using namespace DirectX;

  XMFLOAT3 origin = config.position;

  // X-axis (Blue according to your spec)
  XMFLOAT3 x_end(origin.x + config.length, origin.y, origin.z);
  graphic_->AddDebugLine(origin, x_end, XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));

  // Y-axis (Red according to your spec)
  XMFLOAT3 y_end(origin.x, origin.y + config.length, origin.z);
  graphic_->AddDebugLine(origin, y_end, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));

  // Z-axis (Green according to your spec)
  XMFLOAT3 z_end(origin.x, origin.y, origin.z + config.length);
  graphic_->AddDebugLine(origin, z_end, XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));
}

void DebugDrawer::DrawBox(const DirectX::XMFLOAT3& center, const DirectX::XMFLOAT3& extents, const DirectX::XMFLOAT4& color) {
  using namespace DirectX;

  // Calculate 8 corners
  XMFLOAT3 corners[8] = {
    {center.x - extents.x, center.y - extents.y, center.z - extents.z},  // 0: ---
    {center.x + extents.x, center.y - extents.y, center.z - extents.z},  // 1: +--
    {center.x + extents.x, center.y - extents.y, center.z + extents.z},  // 2: +-+
    {center.x - extents.x, center.y - extents.y, center.z + extents.z},  // 3: --+
    {center.x - extents.x, center.y + extents.y, center.z - extents.z},  // 4: -+-
    {center.x + extents.x, center.y + extents.y, center.z - extents.z},  // 5: ++-
    {center.x + extents.x, center.y + extents.y, center.z + extents.z},  // 6: +++
    {center.x - extents.x, center.y + extents.y, center.z + extents.z},  // 7: -++
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
void DebugDrawer::DrawSphere(const DirectX::XMFLOAT3& center, float radius, const DirectX::XMFLOAT4& color, int segments) {
  assert(segments > 0);
  assert(segments % 2 == 0);
  using namespace DirectX;

  float angle_step = XM_2PI / segments;

  // Draw circles in XY, XZ, and YZ planes
  for (int i = 0; i < segments; ++i) {
    float angle1 = i * angle_step;
    float angle2 = (i + 1) * angle_step;

    // XY plane circle
    {
      XMFLOAT3 p1(center.x + radius * cosf(angle1), center.y + radius * sinf(angle1), center.z);
      XMFLOAT3 p2(center.x + radius * cosf(angle2), center.y + radius * sinf(angle2), center.z);
      graphic_->AddDebugLine(p1, p2, color);
    }

    // XZ plane circle
    {
      XMFLOAT3 p1(center.x + radius * cosf(angle1), center.y, center.z + radius * sinf(angle1));
      XMFLOAT3 p2(center.x + radius * cosf(angle2), center.y, center.z + radius * sinf(angle2));
      graphic_->AddDebugLine(p1, p2, color);
    }

    // YZ plane circle
    {
      XMFLOAT3 p1(center.x, center.y + radius * cosf(angle1), center.z + radius * sinf(angle1));
      XMFLOAT3 p2(center.x, center.y + radius * cosf(angle2), center.z + radius * sinf(angle2));
      graphic_->AddDebugLine(p1, p2, color);
    }
  }
}
