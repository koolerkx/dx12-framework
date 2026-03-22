/**
 * @file debug_draw_service.h
 * @brief Interface for debug line drawing, decoupled from GPU implementation.
 */
#pragma once

#include "Framework/Math/Math.h"

class IDebugDrawService {
 public:
  virtual ~IDebugDrawService() = default;
  virtual void AddDebugLine(const Math::Vector3& start, const Math::Vector3& end, const Math::Vector4& color) = 0;
};
