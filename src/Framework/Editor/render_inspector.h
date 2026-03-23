/**
 * @file render_inspector.h
 * @brief Shared editor inspector helpers for rendering-related component UI patterns.
 */
#pragma once
#if ENABLE_EDITOR

#include "Core/utils.h"
#include "Editor/editor_ui.h"
#include "Font/font_family.h"
#include "Font/text_layout.h"
#include "Math/Math.h"
#include "Render/billboard_type.h"
#include "Render/render_settings.h"
#include "Render/render_types.h"

namespace inspector {

inline void ColorEditor(const char* label, Math::Vector4& color) {
  editor_ui::ColorEdit4(label, &color.x);
}

inline void RenderSettingsEditor(Rendering::RenderSettings& settings, bool show_depth) {
  static const char* BLEND_MODE_NAMES[] = {"Opaque", "AlphaBlend", "Additive", "Premultiplied"};
  int blend = static_cast<int>(settings.blend_mode);
  if (editor_ui::Combo("Blend Mode", &blend, BLEND_MODE_NAMES, 4)) {
    settings.blend_mode = static_cast<Rendering::BlendMode>(blend);
  }

  static const char* SAMPLER_NAMES[] = {"PointWrap", "LinearWrap", "AnisotropicWrap", "PointClamp", "LinearClamp"};
  int sampler = static_cast<int>(settings.sampler_type);
  if (editor_ui::Combo("Sampler", &sampler, SAMPLER_NAMES, 5)) {
    settings.sampler_type = static_cast<Rendering::SamplerType>(sampler);
  }

  if (show_depth) {
    if (editor_ui::Checkbox("Depth Test", &settings.depth_test)) {
      if (!settings.depth_test) settings.depth_write = false;
    }
    editor_ui::BeginDisabled(!settings.depth_test);
    editor_ui::Checkbox("Depth Write", &settings.depth_write);
    editor_ui::EndDisabled();
    editor_ui::Checkbox("Double Sided", &settings.double_sided);
  }
}

inline void BillboardEditor(Billboard::Mode& mode) {
  static const char* BILLBOARD_NAMES[] = {"None", "Cylindrical", "Spherical"};
  int current = static_cast<int>(mode);
  if (editor_ui::Combo("Billboard", &current, BILLBOARD_NAMES, 3)) {
    mode = static_cast<Billboard::Mode>(current);
  }
}

inline void RenderLayerEditor(RenderLayer& layer) {
  static const char* LAYER_NAMES[] = {"Opaque", "Transparent"};
  int current = static_cast<int>(layer);
  if (current >= 2) current = 1;
  if (editor_ui::Combo("Render Layer", &current, LAYER_NAMES, 2)) {
    layer = static_cast<RenderLayer>(current);
  }
}

inline void TextPropertiesEditor(std::wstring& text,
  Font::FontFamily& font_family,
  float& pixel_size,
  Text::HorizontalAlign& h_align,
  float& line_spacing,
  float& letter_spacing) {
  std::string utf8_text = utils::wstring_to_utf8(text);
  char buf[512];
  strncpy_s(buf, utf8_text.c_str(), sizeof(buf) - 1);
  if (editor_ui::InputText("Text", buf, sizeof(buf))) {
    text = utils::utf8_to_wstring(std::string(buf));
  }

  static const char* FONT_NAMES[] = {"ZenOldMincho"};
  int font = static_cast<int>(font_family);
  if (editor_ui::Combo("Font", &font, FONT_NAMES, 1)) {
    font_family = static_cast<Font::FontFamily>(font);
  }

  editor_ui::DragFloat("Pixel Size", &pixel_size, 0.5f, 1.0f, 256.0f);

  static const char* H_ALIGN_NAMES[] = {"Left", "Center", "Right"};
  int h = static_cast<int>(h_align);
  if (editor_ui::Combo("H Align", &h, H_ALIGN_NAMES, 3)) {
    h_align = static_cast<Text::HorizontalAlign>(h);
  }

  editor_ui::DragFloat("Line Spacing", &line_spacing, 0.1f);
  editor_ui::DragFloat("Letter Spacing", &letter_spacing, 0.1f);
}

}  // namespace inspector

#endif
