#include "font_registry.h"

#include <cmath>

namespace Font {

const FontVariant* FontRegistry::RegisterVariant(FontFamily family, BMFont::BmFontData font_data) {
  auto& family_entry = families_[family];
  family_entry.id = family;

  int line_height = font_data.common.lineHeight;

  FontVariant variant;
  variant.native_line_height = static_cast<uint16_t>(line_height);
  variant.font_data = std::move(font_data);

  auto [it, inserted] = family_entry.variants_by_line_height.emplace(line_height, std::move(variant));
  if (!inserted) {
    it->second = std::move(variant);
  }

  return &it->second;
}

const FontVariant* FontRegistry::SelectVariant(FontFamily family, float desired_pixel_size, float& out_scale) const {
  auto family_it = families_.find(family);
  if (family_it == families_.end() || family_it->second.variants_by_line_height.empty()) {
    out_scale = 1.0f;
    return nullptr;
  }

  const auto& variants = family_it->second.variants_by_line_height;

  // Find the closest variant using lower_bound
  auto it = variants.lower_bound(static_cast<int>(desired_pixel_size));

  // If we're past the end, use the largest variant
  if (it == variants.end()) {
    --it;
  }
  // If we're at the beginning and the size is smaller than desired,
  // consider using the next larger one if available
  else if (it != variants.begin()) {
    auto prev_it = std::prev(it);
    // Use the closer one
    int dist_current = std::abs(it->first - static_cast<int>(desired_pixel_size));
    int dist_prev = std::abs(prev_it->first - static_cast<int>(desired_pixel_size));
    if (dist_prev < dist_current) {
      it = prev_it;
    }
  }

  const FontVariant* selected = &it->second;
  out_scale = desired_pixel_size / static_cast<float>(selected->native_line_height);

  return selected;
}

CreateLayoutResult FontRegistry::CreateTextLayout(
  const std::wstring& text, FontFamily family, float pixel_size, const Text::TextLayoutProps& props, TextLayoutData& out_layout) {
  out_layout.glyphs.clear();
  out_layout.width = 0.0f;
  out_layout.height = 0.0f;
  out_layout.font_texture = TextureHandle::Invalid();

  // Select font variant
  float scale = 1.0f;
  const FontVariant* variant = SelectVariant(family, pixel_size, scale);

  if (!variant) {
    return {};
  }

  // Compute layout using text_layout module
  Text::TextLayoutResult layout_result;
  if (!Text::LayoutText(text, variant->font_data, props, layout_result)) {
    return {};
  }

  // Convert to TextLayoutData - pure CPU data with UV coordinates
  out_layout.glyphs.reserve(layout_result.glyphs.size());
  for (const auto& glyph : layout_result.glyphs) {
    TextLayoutData::GlyphData glyph_data;
    glyph_data.x = glyph.x;
    glyph_data.y = glyph.y;
    glyph_data.width = glyph.width;
    glyph_data.height = glyph.height;
    glyph_data.u0 = glyph.u0;
    glyph_data.v0 = glyph.v0;
    glyph_data.u1 = glyph.u1;
    glyph_data.v1 = glyph.v1;
    out_layout.glyphs.push_back(glyph_data);
  }

  out_layout.width = layout_result.width;
  out_layout.height = layout_result.height;

  return {true, variant};
}

}  // namespace Font
