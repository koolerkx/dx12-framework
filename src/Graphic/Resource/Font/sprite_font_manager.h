#pragma once
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Framework/Font/bmfont_parser.h"
#include "Framework/Font/font_types.h"
#include "Framework/Font/text_layout.h"
#include "Framework/Font/text_layout_data.h"
#include "Resource/Texture/texture.h"

class TextureManager;
struct ID3D12Device;

namespace Font {

// Font variant for a specific size
struct SpriteFontVariant {
  uint16_t native_line_height = 0;
  BMFont::BmFontData font_data;
  std::shared_ptr<Texture> texture;
};

// Font family with multiple size variants
struct SpriteFontFamily {
  FontFamily id;
  // Key: native lineHeight, Value: variant
  std::map<int, SpriteFontVariant> variants_by_line_height;
};

// Font manager (internal to Graphic layer)
// Manages font loading and text layout computation
// Does NOT own any per-glyph GPU resources - text rendering uses shared Quad mesh
class SpriteFontManager {
 public:
  bool Initialize(TextureManager* texture_manager, ID3D12Device* device);

  // Load a font variant
  bool LoadFontVariant(FontFamily family, const std::string& fnt_path, const std::string& texture_path);

  // Create text layout - returns pure CPU data (positions, sizes, UVs)
  // Renderer will use shared Quad mesh + UV transform to render each glyph
  bool CreateTextLayout(
    const std::wstring& text, FontFamily family, float pixel_size, const struct Text::TextLayoutProps& props, TextLayoutData& out_layout);

 private:
  // Select the best variant for the desired pixel size
  const SpriteFontVariant* SelectVariant(FontFamily family, float desired_pixel_size, float& out_scale) const;

  TextureManager* texture_manager_ = nullptr;
  ID3D12Device* device_ = nullptr;
  std::map<FontFamily, SpriteFontFamily> families_;
};

void LoadDefaultFonts(SpriteFontManager& sprite_font_manager);

}  // namespace Font
