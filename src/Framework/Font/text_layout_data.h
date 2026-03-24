/**
 * @file text_layout_data.h
 * @brief CPU-only font variant/family data structures and text layout result data.
 */
#pragma once

#include <cstdint>
#include <map>
#include <vector>

#include "Framework/Font/bmfont_parser.h"
#include "Framework/Render/render_handles.h"

namespace Font {

enum class FontFamily : uint16_t {
  ZenOldMincho,
};

// Font variant for a specific size
struct FontVariant {
  uint16_t native_line_height = 0;
  BMFont::BmFontData font_data;
};

// Font family with multiple size variants
struct FontFamilyData {
  FontFamily id;
  // Key: native lineHeight, Value: variant
  std::map<int, FontVariant> variants_by_line_height;
};

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
