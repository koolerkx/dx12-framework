/**
 * @file text_layout_data.h
 * @brief Font text layout result data, shared between Framework and Graphic layers.
 */
#pragma once

#include <vector>

#include "Framework/Render/texture_handle.h"

namespace Font {

struct TextLayoutData {
  struct GlyphData {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 1.0f;
    float v1 = 1.0f;
  };

  std::vector<GlyphData> glyphs;
  float width = 0.0f;
  float height = 0.0f;
  TextureHandle font_texture;
};

}  // namespace Font
