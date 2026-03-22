#include "debug_draw_service.h"

#include "graphic.h"

DebugDrawService::DebugDrawService(Graphic& graphic) : graphic_(graphic) {
}

void DebugDrawService::AddDebugLine(const Math::Vector3& start, const Math::Vector3& end, const Math::Vector4& color) {
  graphic_.AddDebugLine(start, end, color);
}
