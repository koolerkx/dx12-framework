/**
 * @file font_registry.h
 * @brief CPU-side font variant registry with layout computation.
 */
#pragma once

#include <map>
#include <string>

#include "Framework/Font/text_layout.h"
#include "Framework/Font/text_layout_data.h"

namespace Font {

struct CreateLayoutResult {
  bool success = false;
  const FontVariant* selected_variant = nullptr;
};

// Variants are append-only; pointers returned by RegisterVariant() remain stable
// and are used as keys in SpriteFontManager::variant_textures_.
class FontRegistry {
 public:
  // Returns stable pointer to the registered variant (std::map guarantees pointer stability on insert)
  const FontVariant* RegisterVariant(FontFamily family, BMFont::BmFontData font_data);

  // On failure: sets out_scale = 1.0f and returns nullptr
  const FontVariant* SelectVariant(FontFamily family, float desired_pixel_size, float& out_scale) const;

  // On failure: clears out_layout and returns {false, nullptr}
  CreateLayoutResult CreateTextLayout(
    const std::wstring& text, FontFamily family, float pixel_size, const Text::TextLayoutProps& props, TextLayoutData& out_layout);

 private:
  std::map<FontFamily, FontFamilyData> families_;
};

}  // namespace Font
