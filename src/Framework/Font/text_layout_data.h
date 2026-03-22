/**
 * @file text_layout_data.h
 * @brief Font text layout result data, shared between Framework and Graphic layers.
 */
#pragma once

#include <vector>

namespace Font {

struct SpriteFontVariant;

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
  const SpriteFontVariant* variant = nullptr;
};

}  // namespace Font
