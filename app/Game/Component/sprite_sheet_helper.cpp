#include "sprite_sheet_helper.h"
#include <algorithm>
#include <DirectXMath.h>

namespace SpriteSheet {

UVResult CalculateFrameUV(const FrameConfig& config, uint32_t frame_index) {
  using namespace DirectX;

  // Validate frame size doesn't exceed sheet size
  uint32_t available_width = config.sheet_size.x - 2 * config.margin.x;
  uint32_t available_height = config.sheet_size.y - 2 * config.margin.y;

  uint32_t effective_frame_width = config.frame_size.x + config.padding.x;
  uint32_t effective_frame_height = config.frame_size.y + config.padding.y;

  uint32_t frames_per_row = available_width / effective_frame_width;
  uint32_t frames_per_col = available_height / effective_frame_height;

  // Calculate total frames in sheet
  uint32_t total_frames = (config.orientation == Orientation::Horizontal)
    ? frames_per_row * frames_per_col
    : frames_per_col * frames_per_row;

  // Clamp frame_index to valid range [0, total_frames - 1]
  frame_index = std::min(frame_index, total_frames > 0 ? total_frames - 1 : 0);

  // Calculate row and column
  uint32_t row, col;
  if (config.orientation == Orientation::Horizontal) {
    row = frame_index / frames_per_row;
    col = frame_index % frames_per_row;
  } else {
    row = frame_index % frames_per_col;
    col = frame_index / frames_per_col;
  }

  // Calculate pixel position
  uint32_t x = config.margin.x + col * effective_frame_width;
  uint32_t y = config.margin.y + row * effective_frame_height;

  // Convert to normalized UV with half-pixel offset for accurate sampling
  UVResult result;
  result.uv_offset = {
    (static_cast<float>(x) + 0.5f) / config.sheet_size.x,
    (static_cast<float>(y) + 0.5f) / config.sheet_size.y
  };
  result.uv_scale = {
    static_cast<float>(config.frame_size.x) / config.sheet_size.x,
    static_cast<float>(config.frame_size.y) / config.sheet_size.y
  };

  return result;
}

}  // namespace SpriteSheet
