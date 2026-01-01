#include "pivot_type.h"

namespace Pivot {

DirectX::XMFLOAT2 Config::GetNormalized() const {
  switch (preset) {
    case Preset::Center:
      return {0.5f, 0.5f};
    case Preset::Bottom:
      return {0.5f, 1.0f};
    default:
      return {0.5f, 0.5f};
  }
}

DirectX::XMFLOAT2 Config::CalculateOffset(const DirectX::XMFLOAT2& content_size) const {
  // For text rendering:
  // - X axis: glyphs range from 0 to total_max_width (not centered)
  // - Y axis: alignment has centered text around y=0 for Center/Top/Bottom/Baseline
  //
  // So for Center pivot:
  // - X: offset by width/2 to center horizontally
  // - Y: no offset needed because text is already centered by alignment
  //
  // For Bottom pivot:
  // - X: offset by width/2
  // - Y: offset by height/2 to move pivot to bottom

  if (preset == Preset::Center) {
    return {content_size.x * 0.5f, 0.0f};
  }

  // Bottom pivot
  return {content_size.x * 0.5f, content_size.y * 0.5f};
}

}  // namespace Pivot
