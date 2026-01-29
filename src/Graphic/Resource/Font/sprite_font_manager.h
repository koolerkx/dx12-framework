#pragma once
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Framework/Font/bmfont_parser.h"
#include "Framework/Font/text_layout.h"
#include "Resource/Texture/texture.h"

class TextureManager;
struct ID3D12Device;

namespace Font {

// Font family enumeration
enum class FontFamily : uint16_t {
  ZenOldMincho,
  // Add more font families here as needed
};

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

// Result from text layout computation - pure CPU data
// Contains position, size, and UV information for each glyph
// No GPU resource ownership - uses shared Quad mesh from AssetManager
struct TextLayoutData {
  struct GlyphData {
    float x = 0.0f;       // Position X in layout space (pixels)
    float y = 0.0f;       // Position Y in layout space (pixels)
    float width = 0.0f;   // Scaled width (pixels)
    float height = 0.0f;  // Scaled height (pixels)
    float u0 = 0.0f;      // UV left
    float v0 = 0.0f;      // UV top
    float u1 = 1.0f;      // UV right
    float v1 = 1.0f;      // UV bottom
  };

  std::vector<GlyphData> glyphs;
  float width = 0.0f;
  float height = 0.0f;
  const SpriteFontVariant* variant = nullptr;
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
