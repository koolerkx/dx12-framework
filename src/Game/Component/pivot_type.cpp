#include "pivot_type.h"

namespace Pivot {

Vector2 Config::GetNormalized() const {
  switch (preset) {
    case Preset::Center:
      return {0.5f, 0.5f};
    case Preset::Bottom:
      return {0.5f, 1.0f};
    default:
      return {0.5f, 0.5f};
  }
}

Vector2 Config::CalculateOffset(const Vector2& content_size) const {
  if (preset == Preset::Center) {
    return {content_size.x * 0.5f, 0.0f};
  }

  return {content_size.x * 0.5f, content_size.y * 0.5f};
}

}  // namespace Pivot
