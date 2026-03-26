#include "debug_draw_service.h"

#include "Debug/debug_line_renderer.h"
#include "graphic.h"

DebugDrawService::DebugDrawService(Graphic& graphic) : graphic_(graphic) {
}

void DebugDrawService::AddDebugLine(const Math::Vector3& start, const Math::Vector3& end, const Math::Vector4& color) {
  if (auto* r = graphic_.GetDebugLineRenderer()) r->AddLine(start, end, color);
}

void DebugDrawService::AddDebugLine(LineSetId id,
  const Math::Vector3& position,
  const Math::Vector3& axis_x,
  const Math::Vector3& axis_y,
  const Math::Vector3& axis_z,
  const Math::Vector4& color) {
  if (auto* r = graphic_.GetDebugLineRenderer()) r->AddLine(id, position, axis_x, axis_y, axis_z, color);
}
