#pragma once
#include "Framework/Math/Math.h"

using Math::Vector2;

namespace SpriteSheet {

enum class Orientation : uint8_t { Horizontal, Vertical };

struct FrameConfig {
  DirectX::XMUINT2 sheet_size = {0, 0};
  DirectX::XMUINT2 frame_size = {0, 0};
  Orientation orientation = Orientation::Horizontal;
  DirectX::XMUINT2 padding = {0, 0};
  DirectX::XMUINT2 margin = {0, 0};
};

struct UVResult {
  Vector2 uv_offset;
  Vector2 uv_scale;
};

UVResult CalculateFrameUV(const FrameConfig& config, uint32_t frame_index);

}  // namespace SpriteSheet
