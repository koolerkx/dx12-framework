/**
 * @file debug_draw_service.h
 * @brief DebugDrawService implementation delegating to Graphic::AddDebugLine.
 */
#pragma once

#include "Framework/Render/debug_draw_service.h"

class Graphic;

class DebugDrawService : public IDebugDrawService {
 public:
  explicit DebugDrawService(Graphic& graphic);

  void AddDebugLine(const Math::Vector3& start, const Math::Vector3& end, const Math::Vector4& color) override;

 private:
  Graphic& graphic_;
};
