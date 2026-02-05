#pragma once
#include <DirectXMath.h>

#include <string>

#include "Component/component.h"
#include "Component/pivot_type.h"
#include "Component/render_settings.h"
#include "Framework/Font/text_layout.h"
#include "Game/Asset/asset_manager.h"
#include "Graphic/Frame/frame_packet.h"
#include "Graphic/Resource/Font/sprite_font_manager.h"
#include "game_object.h"

class UITextRenderer : public Component<UITextRenderer> {
 public:
  UITextRenderer(GameObject* owner) : Component(owner) {
  }

  void SetText(const std::wstring& text) {
    if (text_ != text) {
      text_ = text;
      dirty_ = true;
    }
  }

  const std::wstring& GetText() const {
    return text_;
  }

  void SetFont(Font::FontFamily family) {
    if (font_family_ != family) {
      font_family_ = family;
      dirty_ = true;
    }
  }

  void SetPixelSize(float size) {
    if (pixel_size_ != size) {
      pixel_size_ = size;
      dirty_ = true;
    }
  }

  void SetColor(const DirectX::XMFLOAT4& color) {
    color_ = color;
  }

  void SetHorizontalAlign(Text::HorizontalAlign align) {
    if (h_align_ != align) {
      h_align_ = align;
      dirty_ = true;
    }
  }

  void SetVerticalAlign(Text::VerticalAlign align) {
    if (v_align_ != align) {
      v_align_ = align;
      dirty_ = true;
    }
  }

  void SetLineSpacing(float spacing) {
    if (line_spacing_ != spacing) {
      line_spacing_ = spacing;
      dirty_ = true;
    }
  }

  void SetLetterSpacing(float spacing) {
    if (letter_spacing_ != spacing) {
      letter_spacing_ = spacing;
      dirty_ = true;
    }
  }

  void SetUseKerning(bool enable) {
    if (use_kerning_ != enable) {
      use_kerning_ = enable;
      dirty_ = true;
    }
  }

  void SetLayerId(int id) {
    layer_id_ = id;
  }

  void SetRenderTags(RenderTagMask tags) {
    render_tags_ = tags;
  }
  void AddRenderTag(RenderTag tag) {
    render_tags_ |= static_cast<uint32_t>(tag);
  }

  void SetBlendMode(Rendering::BlendMode mode) {
    render_settings_.blend_mode = mode;
  }
  void SetSampler(Rendering::SamplerType type) {
    render_settings_.sampler_type = type;
  }
  const Rendering::RenderSettings& GetRenderSettings() const {
    return render_settings_;
  }

  void SetPivot(Pivot::Preset preset);
  void SetPivot(const Pivot::Config& config);
  const Pivot::Config& GetPivot() const {
    return pivot_;
  }

  DirectX::XMFLOAT2 GetSize() const {
    return DirectX::XMFLOAT2(text_mesh_handle_.GetWidth(), text_mesh_handle_.GetHeight());
  }

  void OnRender(FramePacket& packet) override;

 private:
  void RebuildTextMesh(AssetManager& asset_manager);

 private:
  std::wstring text_;
  Font::FontFamily font_family_ = Font::FontFamily::ZenOldMincho;
  float pixel_size_ = 16.0f;
  DirectX::XMFLOAT4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};

  Text::HorizontalAlign h_align_ = Text::HorizontalAlign::Left;
  Text::VerticalAlign v_align_ = Text::VerticalAlign::Baseline;
  float line_spacing_ = 0.0f;
  float letter_spacing_ = 0.0f;
  bool use_kerning_ = true;

  int layer_id_ = 0;
  Rendering::RenderSettings render_settings_ = Rendering::RenderSettings::UI();

  bool dirty_ = true;
  TextMeshHandle text_mesh_handle_;

  Pivot::Config pivot_;
  RenderTagMask render_tags_ = 0;
};
