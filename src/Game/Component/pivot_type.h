#pragma once
#include <DirectXMath.h>

namespace Pivot {

enum class Preset : uint8_t {
  Center,      // Pivot at geometric center (default)
  Bottom,      // Pivot at bottom center (for standing objects)
};

struct Config {
  Preset preset = Preset::Center;

  // Get effective normalized coordinates (0-1)
  // (0.5, 0.5) = center, (0.5, 1.0) = bottom center
  DirectX::XMFLOAT2 GetNormalized() const;

  // Calculate the pivot position in content coordinates
  DirectX::XMFLOAT2 CalculateOffset(const DirectX::XMFLOAT2& content_size) const;
};

}  // namespace Pivot
