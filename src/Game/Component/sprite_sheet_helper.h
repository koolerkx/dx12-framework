#pragma once
#include <DirectXMath.h>

namespace SpriteSheet {

enum class Orientation : uint8_t {
  Horizontal,  // Frames arranged left-to-right
  Vertical     // Frames arranged top-to-bottom
};

struct FrameConfig {
  DirectX::XMUINT2 sheet_size = {0, 0};      // Total sprite sheet dimensions (pixels)
  DirectX::XMUINT2 frame_size = {0, 0};      // Single frame dimensions (pixels)
  Orientation orientation = Orientation::Horizontal;
  DirectX::XMUINT2 padding = {0, 0};  // Spacing between frames (pixels)
  DirectX::XMUINT2 margin = {0, 0};   // Border spacing around sheet (pixels)
};

struct UVResult {
  DirectX::XMFLOAT2 uv_offset;  // Top-left UV coordinate
  DirectX::XMFLOAT2 uv_scale;   // Width/height in UV space (0-1)
};

UVResult CalculateFrameUV(const FrameConfig& config, uint32_t frame_index);

}  // namespace SpriteSheet
