/**
 * @file debug_draw_service.h
 * @brief DebugDrawService implementation delegating to Graphic::AddDebugLine.
 */
#pragma once

#include "Framework/Render/render_service.h"

class Graphic;

class DebugDrawService : public IDebugDrawService {
 public:
  explicit DebugDrawService(Graphic& graphic);

  void AddDebugLine(const Math::Vector3& start, const Math::Vector3& end, const Math::Vector4& color) override;
  std::span<DebugLineVertex> ReserveDebugLines(uint32_t line_count) override;

 private:
  Graphic& graphic_;
};
