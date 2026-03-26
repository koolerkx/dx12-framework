/**
 * @file debug_draw_service.h
 * @brief Pure bridge — forwards IDebugDrawService calls to DebugLineRenderer.
 */
#pragma once

#include "Framework/Render/render_service.h"

class Graphic;

class DebugDrawService : public IDebugDrawService {
 public:
  explicit DebugDrawService(Graphic& graphic);

  void AddDebugLine(const Math::Vector3& start, const Math::Vector3& end, const Math::Vector4& color) override;
  void AddDebugLine(LineSetId id,
    const Math::Vector3& position,
    const Math::Vector3& axis_x,
    const Math::Vector3& axis_y,
    const Math::Vector3& axis_z,
    const Math::Vector4& color) override;

 private:
  Graphic& graphic_;
};
