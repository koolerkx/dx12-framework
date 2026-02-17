#pragma once
#include <optional>
#include <string>

#include "Component/component.h"
#include "Component/pivot_type.h"
#include "Component/render_settings.h"
#include "Framework/Core/utils.h"
#include "Framework/Font/text_layout.h"
#include "Framework/Math/Math.h"
#include "Framework/Serialize/serialize_node.h"
#include "Game/Asset/asset_manager.h"
#include "Graphic/Frame/frame_packet.h"
#include "Graphic/Resource/Font/sprite_font_manager.h"
#include "game_object.h"

using Math::Vector2;
using Math::Vector4;

class UITextRenderer : public Component<UITextRenderer> {
 public:
  struct Props {
    std::wstring text;
    Font::FontFamily font = Font::FontFamily::ZenOldMincho;
    float pixel_size = 16.0f;
    Vector4 color = {1, 1, 1, 1};
    Text::HorizontalAlign h_align = Text::HorizontalAlign::Left;
    Text::VerticalAlign v_align = Text::VerticalAlign::Baseline;
    Vector2 pivot = {0.5f, 0.0f};
    std::optional<int> layer_id = std::nullopt;
  };

  UITextRenderer(GameObject* owner) : Component(owner) {
  }

  UITextRenderer(GameObject* owner, const Props& props) : Component(owner) {
    if (!props.text.empty()) SetText(props.text);
    SetFont(props.font);
    SetPixelSize(props.pixel_size);
    SetColor(props.color);

    SetHorizontalAlign(props.h_align);
    SetVerticalAlign(props.v_align);

    SetPivot(props.pivot);
    layer_id_ = props.layer_id.value_or(InheritParentUILayerId());
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

  void SetColor(const Vector4& color) {
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

  std::optional<int> GetUILayerId() const override {
    return layer_id_;
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
  void SetPivot(const Vector2& normalized_pivot);
  const Vector2& GetPivot() const {
    return text_pivot_;
  }

  Vector2 GetSize() const {
    return Vector2(text_mesh_handle_.GetWidth(), text_mesh_handle_.GetHeight());
  }

  void OnSerialize(framework::SerializeNode& node) const override {
    node.Write("Text", utils::wstring_to_utf8(text_));
    node.Write("FontFamily", static_cast<int>(font_family_));
    node.Write("PixelSize", pixel_size_);
    node.WriteVec4("Color", color_.x, color_.y, color_.z, color_.w);

    node.Write("HAlign", static_cast<int>(h_align_));
    node.Write("VAlign", static_cast<int>(v_align_));
    node.Write("LineSpacing", line_spacing_);
    node.Write("LetterSpacing", letter_spacing_);
    node.Write("UseKerning", use_kerning_);

    node.Write("LayerId", layer_id_);
    node.WriteVec2("Pivot", text_pivot_.x, text_pivot_.y);
  }

  void OnDeserialize(const framework::SerializeNode& node) override {
    auto utf8 = node.ReadString("Text");
    if (!utf8.empty()) SetText(utils::utf8_to_wstring(utf8));
    SetFont(static_cast<Font::FontFamily>(node.ReadInt("FontFamily", static_cast<int>(font_family_))));
    SetPixelSize(node.ReadFloat("PixelSize", pixel_size_));
    node.ReadVec4("Color", color_.x, color_.y, color_.z, color_.w);

    SetHorizontalAlign(static_cast<Text::HorizontalAlign>(node.ReadInt("HAlign", static_cast<int>(h_align_))));
    SetVerticalAlign(static_cast<Text::VerticalAlign>(node.ReadInt("VAlign", static_cast<int>(v_align_))));
    SetLineSpacing(node.ReadFloat("LineSpacing", line_spacing_));
    SetLetterSpacing(node.ReadFloat("LetterSpacing", letter_spacing_));
    SetUseKerning(node.ReadBool("UseKerning", use_kerning_));

    SetLayerId(node.ReadInt("LayerId", layer_id_));
    node.ReadVec2("Pivot", text_pivot_.x, text_pivot_.y);
  }

  struct EditorData {
    std::wstring text;
    Font::FontFamily font_family;
    float pixel_size;
    Vector4 color;
    Text::HorizontalAlign h_align;
    Text::VerticalAlign v_align;
    float line_spacing;
    float letter_spacing;
    bool use_kerning;
    int layer_id;
    Vector2 pivot;
    Rendering::RenderSettings render_settings;
    RenderTagMask render_tags;
  };

  EditorData GetEditorData() const;
  void ApplyEditorData(const EditorData& data);

  void OnRender(FramePacket& packet) override;

 private:
  void RebuildTextMesh(AssetManager& asset_manager);

 private:
  std::wstring text_;
  Font::FontFamily font_family_ = Font::FontFamily::ZenOldMincho;
  float pixel_size_ = 16.0f;
  Vector4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};

  Text::HorizontalAlign h_align_ = Text::HorizontalAlign::Left;
  Text::VerticalAlign v_align_ = Text::VerticalAlign::Baseline;
  float line_spacing_ = 0.0f;
  float letter_spacing_ = 0.0f;
  bool use_kerning_ = true;

  int layer_id_ = 0;
  Rendering::RenderSettings render_settings_ = Rendering::RenderSettings::UI();

  bool dirty_ = true;
  TextMeshHandle text_mesh_handle_;

  Vector2 text_pivot_ = {0.5f, 0.0f};
  RenderTagMask render_tags_ = 0;
};
