/**
 * @file font_service.h
 * @brief Interface for font loading and text layout, decoupled from GPU implementation.
 */
#pragma once

#include <string>

#include "Framework/Font/text_layout_data.h"

namespace Text {
struct TextLayoutProps;
}

class IFontService {
 public:
  virtual ~IFontService() = default;
  virtual bool LoadFontVariant(Font::FontFamily family, const std::string& fnt, const std::string& tex) = 0;
  virtual bool CreateTextLayout(const std::wstring& text,
    Font::FontFamily family,
    float pixel_size,
    const Text::TextLayoutProps& props,
    Font::TextLayoutData& out_layout) = 0;
};
