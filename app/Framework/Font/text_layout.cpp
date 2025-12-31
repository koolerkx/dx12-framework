#include "text_layout.h"

#include <algorithm>
#include <limits>

namespace Text {

namespace {

const float FLOAT_INF = std::numeric_limits<float>::max();
const float FLOAT_NEG_INF = std::numeric_limits<float>::lowest();

// Helper: Find character or fallback to '?' or space
const BMFont::BmFontChar* FindChar(const BMFont::BmFontData& font, uint32_t id) {
  auto it = font.chars.find(id);
  if (it != font.chars.end()) {
    return &it->second;
  }

  // Fallback to '?'
  it = font.chars.find(static_cast<uint32_t>('?'));
  if (it != font.chars.end()) {
    return &it->second;
  }

  // Fallback to space
  it = font.chars.find(static_cast<uint32_t>(' '));
  if (it != font.chars.end()) {
    return &it->second;
  }

  return nullptr;
}

}  // namespace

bool LayoutText(const std::wstring& text, const BMFont::BmFontData& font_data, const TextLayoutProps& props, TextLayoutResult& out_result) {
  out_result.glyphs.clear();
  out_result.width = 0.0f;
  out_result.height = 0.0f;
  out_result.origin_x = 0.0f;
  out_result.origin_y = 0.0f;

  if (text.empty() || font_data.common.lineHeight == 0) {
    return false;
  }

  // Calculate scale factor
  const float scale = props.pixel_size / static_cast<float>(font_data.common.lineHeight);
  out_result.scale = scale;

  const float line_height = static_cast<float>(font_data.common.lineHeight) * scale;
  const float scale_w = static_cast<float>(font_data.common.scaleW);
  const float scale_h = static_cast<float>(font_data.common.scaleH);

  // Structure to track line information for multi-line alignment
  struct LineInfo {
    size_t start_glyph_index = 0;
    size_t end_glyph_index = 0;
    float width = 0.0f;
    float min_x = FLOAT_INF;
    float max_x = FLOAT_NEG_INF;
  };
  std::vector<LineInfo> lines;
  LineInfo current_line;
  current_line.start_glyph_index = 0;

  // Layout state
  float cursor_x = 0.0f;
  float cursor_y = 0.0f;
  float total_min_y = FLOAT_INF;
  float total_max_y = FLOAT_NEG_INF;
  float total_max_width = 0.0f;

  uint32_t prev_char_id = 0;

  // Process each character
  for (size_t i = 0; i < text.size(); ++i) {
    wchar_t wc = text[i];
    uint32_t char_id = static_cast<uint32_t>(wc);

    // Handle newline
    if (wc == L'\n') {
      // Finish current line
      current_line.end_glyph_index = out_result.glyphs.size();
      current_line.width = current_line.max_x - (current_line.min_x == FLOAT_INF ? 0.0f : current_line.min_x);
      if (current_line.min_x == FLOAT_INF) {
        current_line.min_x = 0.0f;
        current_line.max_x = 0.0f;
      }
      lines.push_back(current_line);
      total_max_width = std::max(total_max_width, current_line.width);

      // Start new line
      cursor_x = 0.0f;
      cursor_y += line_height + props.line_spacing * scale;
      current_line = LineInfo();
      current_line.start_glyph_index = out_result.glyphs.size();
      prev_char_id = 0;
      continue;
    }

    // Find character data
    const BMFont::BmFontChar* glyph = FindChar(font_data, char_id);
    if (!glyph) {
      prev_char_id = 0;
      continue;
    }

    // Apply kerning if enabled
    if (props.use_kerning && prev_char_id != 0) {
      int16_t kern = font_data.GetKerning(prev_char_id, char_id);
      cursor_x += static_cast<float>(kern) * scale;
    }

    // Calculate glyph position and size
    const float glyph_x = cursor_x + static_cast<float>(glyph->xoffset) * scale;
    const float glyph_y = cursor_y + static_cast<float>(glyph->yoffset) * scale;
    const float glyph_w = static_cast<float>(glyph->width) * scale;
    const float glyph_h = static_cast<float>(glyph->height) * scale;

    // Update line bounding box
    if (glyph->width > 0 && glyph->height > 0) {
      current_line.min_x = std::min(current_line.min_x, glyph_x);
      current_line.max_x = std::max(current_line.max_x, glyph_x + glyph_w);
      total_min_y = std::min(total_min_y, glyph_y);
      total_max_y = std::max(total_max_y, glyph_y + glyph_h);
    }

    // Calculate UV coordinates
    // Quad mesh UV: top-left (0,0), bottom-left (0,1)
    // BMFont: y=0 is at texture top, so glyph->y is the TOP edge
    // v0 = top of glyph, v1 = bottom of glyph (to match Quad UV layout)
    const float u0 = static_cast<float>(glyph->x) / scale_w;
    const float u1 = static_cast<float>(glyph->x + glyph->width) / scale_w;
    const float v0 = static_cast<float>(glyph->y + glyph->height) / scale_h;
    const float v1 = static_cast<float>(glyph->y) / scale_h;

    // Create glyph instance
    GlyphInstance instance;
    instance.x = glyph_x;
    instance.y = glyph_y;
    instance.width = glyph_w;
    instance.height = glyph_h;
    instance.u0 = u0;
    instance.v0 = v0;
    instance.u1 = u1;
    instance.v1 = v1;
    instance.char_id = char_id;

    out_result.glyphs.push_back(instance);

    // Advance cursor
    cursor_x += static_cast<float>(glyph->xadvance) * scale + props.letter_spacing * scale;
    prev_char_id = char_id;
  }

  // Finish last line
  current_line.end_glyph_index = out_result.glyphs.size();
  current_line.width = current_line.max_x - (current_line.min_x == FLOAT_INF ? 0.0f : current_line.min_x);
  if (current_line.min_x == FLOAT_INF) {
    current_line.min_x = 0.0f;
    current_line.max_x = 0.0f;
  }
  lines.push_back(current_line);
  total_max_width = std::max(total_max_width, current_line.width);

  // Calculate final dimensions
  const float baseline_bottom = cursor_y + line_height;

  if (total_min_y == FLOAT_INF) {
    // No visible characters
    out_result.width = 0.0f;
    out_result.height = baseline_bottom;
  } else {
    out_result.width = total_max_width;
    out_result.height = std::max(total_max_y - total_min_y, baseline_bottom);
  }

  // Apply horizontal alignment PER LINE (multi-line text alignment)
  // This aligns text WITHIN the text block, not the text block position
  for (const auto& line : lines) {
    float line_offset_x = 0.0f;

    switch (props.h_align) {
      case HorizontalAlign::Left:
        // Align to left edge (x=0)
        line_offset_x = -line.min_x;
        break;
      case HorizontalAlign::Center:
        // Center this line within the total width
        line_offset_x = (total_max_width - line.width) * 0.5f - line.min_x;
        break;
      case HorizontalAlign::Right:
        // Align to right edge (x = total_max_width)
        line_offset_x = total_max_width - line.max_x;
        break;
    }

    // Apply offset to all glyphs in this line
    for (size_t i = line.start_glyph_index; i < line.end_glyph_index; ++i) {
      out_result.glyphs[i].x += line_offset_x;
    }
  }

  // Vertical alignment - applies to the entire text block
  float v_offset = 0.0f;
  switch (props.v_align) {
    case VerticalAlign::Top:
      v_offset = -total_min_y;
      break;
    case VerticalAlign::Center:
      v_offset = -(total_min_y + total_max_y) * 0.5f;
      break;
    case VerticalAlign::Bottom:
      v_offset = -baseline_bottom;
      break;
    case VerticalAlign::Baseline:
      v_offset = 0.0f;
      break;
  }

  // Apply vertical offset to all glyphs
  for (auto& glyph : out_result.glyphs) {
    glyph.y += v_offset;
  }

  out_result.origin_x = 0.0f;  // Alignment is already applied to glyphs
  out_result.origin_y = v_offset;

  return true;
}

}  // namespace Text
