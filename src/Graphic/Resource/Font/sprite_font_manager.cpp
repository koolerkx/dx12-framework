#include "sprite_font_manager.h"

#include "Framework/Logging/logger.h"
#include "Resource/Texture/texture.h"
#include "Resource/Texture/texture_manager.h"

namespace Font {

bool SpriteFontManager::Initialize(TextureManager* texture_manager, ID3D12Device* device) {
  texture_manager_ = texture_manager;
  device_ = device;
  return texture_manager_ != nullptr && device_ != nullptr;
}

bool SpriteFontManager::LoadFontVariant(FontFamily family, const std::string& fnt_path, const std::string& texture_path) {
  if (!texture_manager_) {
    return false;
  }

  BMFont::BmFontData font_data;
  if (!BMFont::ParseBmFontText(fnt_path, font_data)) {
    return false;
  }

  // Load texture BEFORE registering variant — if texture fails, registry stays clean
  auto texture = texture_manager_->LoadTexture(texture_path);
  if (!texture) {
    return false;
  }

  const FontVariant* variant = font_registry_.RegisterVariant(family, std::move(font_data));
  variant_textures_[variant] = std::move(texture);

  return true;
}

bool SpriteFontManager::CreateTextLayout(
  const std::wstring& text, FontFamily family, float pixel_size, const Text::TextLayoutProps& props, TextLayoutData& out_layout) {
  auto result = font_registry_.CreateTextLayout(text, family, pixel_size, props, out_layout);

  if (!result.success) {
    return false;
  }

  auto tex_it = variant_textures_.find(result.selected_variant);
  if (tex_it == variant_textures_.end() || !tex_it->second) {
    return false;
  }

  out_layout.font_texture = TextureHandle{tex_it->second->GetBindlessIndex()};
  return true;
}

void LoadDefaultFonts(SpriteFontManager& sprite_font_manager) {
  std::string base_path = "Content/font/";

  bool success = sprite_font_manager.LoadFontVariant(
    Font::FontFamily::ZenOldMincho, base_path + "Zen_Old_Mincho_32px.fnt", base_path + "Zen_Old_Mincho_32px_0.png");

  if (!success) {
    Logger::LogFormat(LogLevel::Warn, LogCategory::Resource, Logger::Here(), "Warning: Failed to load default font");
  }
}

}  // namespace Font
