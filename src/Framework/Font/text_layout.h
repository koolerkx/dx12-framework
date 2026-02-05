#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "bmfont_parser.h"

namespace Text {

// Alignment enums
enum class HorizontalAlign { Left, Center, Right };

enum class VerticalAlign { Top, Center, Bottom, Baseline };

// Layout properties
struct TextLayoutProps {
  float pixel_size = 16.0f;     // Desired text height in pixels
  float line_spacing = 0.0f;    // Additional line spacing (pixels)
  float letter_spacing = 0.0f;  // Additional letter spacing (pixels)
  HorizontalAlign h_align = HorizontalAlign::Left;
  VerticalAlign v_align = VerticalAlign::Baseline;
  bool use_kerning = true;  // Enable kerning if available
};

// Single glyph instance ready for rendering
struct GlyphInstance {
  float x = 0.0f;       // Position X (in layout space)
  float y = 0.0f;       // Position Y (in layout space)
  float width = 0.0f;   // Scaled width
  float height = 0.0f;  // Scaled height
  float u0 = 0.0f;      // UV coordinates
  float v0 = 0.0f;
  float u1 = 0.0f;
  float v1 = 0.0f;
  uint32_t char_id = 0;  // Character ID (for debugging)
};

// Layout result
struct TextLayoutResult {
  std::vector<GlyphInstance> glyphs;
  float width = 0.0f;     // Bounding box width
  float height = 0.0f;    // Bounding box height
  float origin_x = 0.0f;  // Origin offset for alignment
  float origin_y = 0.0f;
  float scale = 1.0f;  // Applied scale factor
};

// Main layout function
bool LayoutText(const std::wstring& text, const BMFont::BmFontData& font_data, const TextLayoutProps& props, TextLayoutResult& out_result);

}  // namespace Text
