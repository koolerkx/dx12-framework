/**
 * @file sprite_font_manager.h
 * @brief GPU-side font resource manager, delegates CPU logic to FontRegistry.
 */
#pragma once

#include <map>
#include <memory>
#include <string>

#include "Framework/Font/font_family.h"
#include "Framework/Font/font_registry.h"

class TextureManager;
struct Texture;
struct ID3D12Device;

namespace Font {

// Does NOT own any per-glyph GPU resources - text rendering uses shared Quad mesh
class SpriteFontManager {
 public:
  bool Initialize(TextureManager* texture_manager, ID3D12Device* device);

  bool LoadFontVariant(FontFamily family, const std::string& fnt_path, const std::string& texture_path);

  bool CreateTextLayout(
    const std::wstring& text, FontFamily family, float pixel_size, const struct Text::TextLayoutProps& props, TextLayoutData& out_layout);

 private:
  TextureManager* texture_manager_ = nullptr;
  ID3D12Device* device_ = nullptr;
  FontRegistry font_registry_;
  std::map<const FontVariant*, std::shared_ptr<Texture>> variant_textures_;
};

void LoadDefaultFonts(SpriteFontManager& sprite_font_manager);

}  // namespace Font
