#pragma once
#include "Framework/Math/Math.h"

using Math::Vector2;

namespace Pivot {

enum class Preset : uint8_t {
  Center,
  Bottom,
};

struct Config {
  Preset preset = Preset::Center;

  Vector2 GetNormalized() const;
  Vector2 CalculateOffset(const Vector2& content_size) const;
};

}  // namespace Pivot
