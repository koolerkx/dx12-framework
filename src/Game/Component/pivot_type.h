#pragma once
#include "Framework/Math/Math.h"

using Math::Vector2;

namespace Pivot {

enum class Preset : uint8_t {
  Center,
  Bottom,
};

Vector2 ToSpriteNormalized(Preset preset);
Vector2 ToUISpriteNormalized(Preset preset);
Vector2 ToTextNormalized(Preset preset);

}  // namespace Pivot
