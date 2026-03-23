/**
 * @file font_family.h
 * @brief CPU-only font variant and family data structures.
 */
#pragma once

#include <cstdint>
#include <map>

#include "Framework/Font/bmfont_parser.h"

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

}  // namespace Font
