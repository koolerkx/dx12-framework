/**
 * @file font_service.h
 * @brief FontService implementation delegating to SpriteFontManager.
 */
#pragma once

#include "Framework/Asset/font_service.h"

namespace Font {
class SpriteFontManager;
}

class FontService : public IFontService {
 public:
  explicit FontService(Font::SpriteFontManager& font_manager);

  bool LoadFontVariant(Font::FontFamily family, const std::string& fnt, const std::string& tex) override;
  bool CreateTextLayout(const std::wstring& text, Font::FontFamily family, float pixel_size,
    const Text::TextLayoutProps& props, Font::TextLayoutData& out_layout) override;
  TextureHandle GetFontTextureHandle(const Font::TextLayoutData& layout) const override;

 private:
  Font::SpriteFontManager& font_manager_;
};
