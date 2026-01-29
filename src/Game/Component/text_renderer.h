#pragma once
#include <DirectXMath.h>

#include <string>

#include "Component/billboard_type.h"
#include "Component/component.h"
#include "Component/pivot_type.h"
#include "Component/render_settings.h"
#include "Framework/Font/text_layout.h"
#include "Game/Asset/asset_manager.h"
#include "Graphic/Frame/frame_packet.h"
#include "Graphic/Resource/Font/sprite_font_manager.h"
#include "game_object.h"

class TextRenderer : public Component<TextRenderer> {
  // Position using transform component, always at left side component

 public:
  TextRenderer(GameObject* owner) : Component(owner) {
  }

  // Text Content
  void SetText(const std::wstring& text) {
    if (text_ != text) {
      text_ = text;
      dirty_ = true;
    }
  }

  const std::wstring& GetText() const {
    return text_;
  }

  // Font Properties
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

  // Layout Properties
  // Text Layout
  void SetHorizontalAlign(Text::HorizontalAlign align) {
    if (h_align_ != align) {
      h_align_ = align;
      dirty_ = true;
    }
  }

  // Pivot point to transform component
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

  // Render Properties
  // This will override render settings
  void SetRenderPassTag(RenderPassTag tag) {
    pass_tag_ = tag;

    switch (tag) {
      case RenderPassTag::Ui:
        render_settings_ = Rendering::RenderSettings::UI();
        break;
      case RenderPassTag::WorldOpaque:
        render_settings_ = Rendering::RenderSettings::Opaque();
        break;
      case RenderPassTag::WorldTransparent:
        render_settings_ = Rendering::RenderSettings::Transparent();
        break;
      default:
        break;
    }

    SetDoubleSided(true);
    SetSampler(Rendering::SamplerType::LinearWrap);
  }

  void SetLayerId(int id) {
    layer_id_ = id;
  }

  // Render Settings API
  void SetBlendMode(Rendering::BlendMode mode) {
    render_settings_.blend_mode = mode;
  }
  void SetSampler(Rendering::SamplerType type) {
    render_settings_.sampler_type = type;
  }
  void SetDepthTest(bool enabled) {
    render_settings_.depth_test = enabled;
  }
  void SetDepthWrite(bool enabled) {
    render_settings_.depth_write = enabled;
  }
  void SetDoubleSided(bool enabled) {
    render_settings_.double_sided = enabled;
  }
  const Rendering::RenderSettings& GetRenderSettings() const {
    return render_settings_;
  }

  // Billboard mode API
  void SetBillboardMode(Billboard::Mode mode) {
    billboard_mode_ = mode;
  }
  Billboard::Mode GetBillboardMode() const {
    return billboard_mode_;
  }

  // Pivot API
  // Note: SetHorizontalAlign controls multi-line text layout alignment (per-line)
  //       SetVerticalAlign controls text baseline position (entire block)
  //       SetPivot controls transform origin point (where the object rotates around)
  void SetPivot(Pivot::Preset preset);
  void SetPivot(const Pivot::Config& config);
  const Pivot::Config& GetPivot() const {
    return pivot_;
  }

  // Query
  DirectX::XMFLOAT2 GetSize() const {
    return DirectX::XMFLOAT2(text_mesh_handle_.GetWidth(), text_mesh_handle_.GetHeight());
  }

  DirectX::XMMATRIX GetBillboardWorldMatrix(const CameraData& camera) const;

  // Rendering
  void OnRender(FramePacket& packet) override;

 private:
  void RebuildTextMesh(AssetManager& asset_manager);
  DirectX::XMMATRIX CalculateBaseWorldMatrix(TransformComponent* transform, const CameraData& camera) const;

 private:
  // Text properties
  std::wstring text_;
  Font::FontFamily font_family_ = Font::FontFamily::ZenOldMincho;
  float pixel_size_ = 16.0f;
  DirectX::XMFLOAT4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};

  // Layout properties
  Text::HorizontalAlign h_align_ = Text::HorizontalAlign::Left;
  Text::VerticalAlign v_align_ = Text::VerticalAlign::Baseline;
  float line_spacing_ = 0.0f;
  float letter_spacing_ = 0.0f;
  bool use_kerning_ = true;

  // Render properties
  RenderPassTag pass_tag_ = RenderPassTag::Ui;
  int layer_id_ = 0;
  Rendering::RenderSettings render_settings_ = Rendering::RenderSettings::UI();

  // Cached text mesh (managed by AssetManager)
  bool dirty_ = true;
  TextMeshHandle text_mesh_handle_;

  Billboard::Mode billboard_mode_ = Billboard::Mode::None;
  Pivot::Config pivot_;
};
