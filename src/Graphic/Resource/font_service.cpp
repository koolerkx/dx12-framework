#include "font_service.h"

#include "Font/sprite_font_manager.h"
#include "Texture/texture.h"

FontService::FontService(Font::SpriteFontManager& font_manager) : font_manager_(font_manager) {
}

bool FontService::LoadFontVariant(Font::FontFamily family, const std::string& fnt, const std::string& tex) {
  return font_manager_.LoadFontVariant(family, fnt, tex);
}

bool FontService::CreateTextLayout(const std::wstring& text, Font::FontFamily family, float pixel_size,
  const Text::TextLayoutProps& props, Font::TextLayoutData& out_layout) {
  return font_manager_.CreateTextLayout(text, family, pixel_size, props, out_layout);
}

TextureHandle FontService::GetFontTextureHandle(const Font::TextLayoutData& layout) const {
  if (layout.variant && layout.variant->texture) {
    return TextureHandle{layout.variant->texture->GetBindlessIndex()};
  }
  return TextureHandle::Invalid();
}
