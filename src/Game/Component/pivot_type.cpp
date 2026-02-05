#include "pivot_type.h"

namespace Pivot {

Vector2 ToSpriteNormalized(Preset preset) {
  switch (preset) {
    case Preset::Center:
      return {0.5f, 0.5f};
    case Preset::Bottom:
      return {0.5f, 1.0f};
    default:
      return {0.5f, 0.5f};
  }
}

Vector2 ToUISpriteNormalized(Preset preset) {
  switch (preset) {
    case Preset::Center:
      return {0.5f, 0.5f};
    case Preset::Bottom:
      return {0.5f, 1.0f};
    default:
      return {0.0f, 0.0f};
  }
}

Vector2 ToTextNormalized(Preset preset) {
  switch (preset) {
    case Preset::Center:
      return {0.5f, 0.0f};
    case Preset::Bottom:
      return {0.5f, 0.5f};
    default:
      return {0.5f, 0.0f};
  }
}

}  // namespace Pivot
