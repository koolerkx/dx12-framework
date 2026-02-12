#pragma once
#include <optional>

#include "Asset/asset_handle.h"
#include "Component/billboard_type.h"
#include "Component/component.h"
#include "Component/pivot_type.h"
#include "Component/render_settings.h"
#include "Component/sprite_sheet_animator.h"
#include "Component/transform_component.h"
#include "Framework/Math/Math.h"
#include "Framework/Serialize/serialize_node.h"
#include "Game/Asset/asset_manager.h"
#include "Graphic/Frame/frame_packet.h"
#include "Graphic/Resource/Texture/texture.h"
#include "game_context.h"
#include "game_object.h"

using Math::Matrix4;
using Math::Vector2;
using Math::Vector3;
using Math::Vector4;

class SpriteRenderer : public Component<SpriteRenderer> {
 public:
  struct Props {
    std::string texture_path;
    Vector4 color = {1, 1, 1, 1};
    Vector2 size = {100, 100};
    Billboard::Mode billboard_mode = Billboard::Mode::None;
    Rendering::BlendMode blend_mode = Rendering::BlendMode::AlphaBlend;
    Vector2 pivot = {0.5f, 0.5f};
    bool double_sided = false;
  };

  SpriteRenderer(GameObject* owner) : Component(owner) {
  }

  SpriteRenderer(GameObject* owner, const Props& props) : Component(owner) {
    if (!props.texture_path.empty()) SetTexturePath(props.texture_path);
    SetColor(props.color);
    SetSize(props.size);
    SetBillboardMode(props.billboard_mode);
    SetBlendMode(props.blend_mode);
    SetPivot(props.pivot);
    if (props.double_sided) SetDoubleSided(true);
  }

  void SetTexture(Texture* tex) {
    texture_ = tex;
    texture_path_.clear();
    texture_handle_ = {};
  }

  void SetTexturePath(const std::string& path) {
    texture_path_ = path;
    if (path.empty()) {
      texture_ = nullptr;
      texture_handle_ = {};
      return;
    }
    auto* context = GetOwner()->GetContext();
    if (!context) return;
    texture_handle_ = context->GetAssetManager().LoadTexture(path);
    texture_ = texture_handle_.Get();
  }
  void SetColor(const Vector4& color) {
    color_ = color;
  }
  void SetSize(const Vector2& size) {
    size_ = size;
  }

  void SetRenderLayer(RenderLayer layer);

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

  void SetBillboardMode(Billboard::Mode mode) {
    billboard_mode_ = mode;
  }
  Billboard::Mode GetBillboardMode() const {
    return billboard_mode_;
  }

  void SetPivot(Pivot::Preset preset);
  void SetPivot(const Vector2& normalized_pivot);
  const Vector2& GetPivot() const {
    return sprite_pivot_;
  }

  void SetUVOffset(const Vector2& offset) {
    uv_offset_ = offset;
  }
  void SetUVScale(const Vector2& scale) {
    uv_scale_ = scale;
  }
  const Vector2& GetUVOffset() const {
    return uv_offset_;
  }
  const Vector2& GetUVScale() const {
    return uv_scale_;
  }

  void OnSerialize(framework::SerializeNode& node) const override {
    if (!texture_path_.empty()) {
      node.Write("Texture", texture_path_);
    }
    node.WriteVec4("Color", color_.x, color_.y, color_.z, color_.w);
    node.WriteVec2("Size", size_.x, size_.y);
    node.WriteVec2("Pivot", sprite_pivot_.x, sprite_pivot_.y);
    node.WriteVec2("UVOffset", uv_offset_.x, uv_offset_.y);
    node.WriteVec2("UVScale", uv_scale_.x, uv_scale_.y);
    node.Write("BillboardMode", static_cast<int>(billboard_mode_));
    node.Write("BlendMode", static_cast<int>(render_settings_.blend_mode));
    node.Write("SamplerType", static_cast<int>(render_settings_.sampler_type));
    node.Write("DepthTest", render_settings_.depth_test);
    node.Write("DepthWrite", render_settings_.depth_write);
    node.Write("DoubleSided", render_settings_.double_sided);
    node.Write("RenderTags", static_cast<int>(render_tags_));
    node.Write("RenderLayer", render_layer_ == RenderLayer::Opaque ? "Opaque" : "Transparent");
    if (animator_) {
      auto anim_node = node.BeginMap("Animator");
      animator_->Serialize(anim_node);
    }
  }

  void OnDeserialize(const framework::SerializeNode& node) override {
    auto tex_path = node.ReadString("Texture");
    if (!tex_path.empty()) {
      SetTexturePath(tex_path);
    }
    node.ReadVec4("Color", color_.x, color_.y, color_.z, color_.w);
    node.ReadVec2("Size", size_.x, size_.y);
    node.ReadVec2("Pivot", sprite_pivot_.x, sprite_pivot_.y);
    node.ReadVec2("UVOffset", uv_offset_.x, uv_offset_.y);
    node.ReadVec2("UVScale", uv_scale_.x, uv_scale_.y);
    billboard_mode_ = static_cast<Billboard::Mode>(node.ReadInt("BillboardMode", static_cast<int>(billboard_mode_)));
    render_settings_.blend_mode =
      static_cast<Rendering::BlendMode>(node.ReadInt("BlendMode", static_cast<int>(render_settings_.blend_mode)));
    render_settings_.sampler_type =
      static_cast<Rendering::SamplerType>(node.ReadInt("SamplerType", static_cast<int>(render_settings_.sampler_type)));
    render_settings_.depth_test = node.ReadBool("DepthTest", render_settings_.depth_test);
    render_settings_.depth_write = node.ReadBool("DepthWrite", render_settings_.depth_write);
    render_settings_.double_sided = node.ReadBool("DoubleSided", render_settings_.double_sided);
    render_tags_ = static_cast<RenderTagMask>(node.ReadInt("RenderTags", static_cast<int>(render_tags_)));
    auto layer_str = node.ReadString("RenderLayer", "Transparent");
    render_layer_ = (layer_str == "Opaque") ? RenderLayer::Opaque : RenderLayer::Transparent;
    if (node.HasKey("Animator")) {
      auto anim_node = node.GetMap("Animator");
      GetAnimator().Deserialize(anim_node);
    }
  }

  struct EditorData {
    Vector4 color;
    Vector2 size;
    Vector2 pivot;
    Vector2 uv_offset;
    Vector2 uv_scale;
    Billboard::Mode billboard_mode;
    Rendering::RenderSettings render_settings;
    RenderLayer render_layer;
    RenderTagMask render_tags;
  };

  EditorData GetEditorData() const;
  void ApplyEditorData(const EditorData& data);

  SpriteSheetAnimator& GetAnimator();

  void OnUpdate(float dt) override;
  void OnRender(FramePacket& packet) override;

 private:
  Matrix4 CalculateWorldMatrix(TransformComponent* transform, const CameraData& camera) const;

 private:
  Texture* texture_ = nullptr;
  std::string texture_path_;
  AssetHandle<Texture> texture_handle_;
  Vector4 color_ = {1, 1, 1, 1};
  Vector2 size_ = {100, 100};
  Vector2 uv_offset_ = {0.0f, 0.0f};
  Vector2 uv_scale_ = {1.0f, 1.0f};

  Rendering::RenderSettings render_settings_ = Rendering::RenderSettings::Transparent();
  Billboard::Mode billboard_mode_ = Billboard::Mode::None;
  Vector2 sprite_pivot_ = {0.5f, 0.5f};

  RenderLayer render_layer_ = RenderLayer::Transparent;
  RenderTagMask render_tags_ = 0;

  std::optional<SpriteSheetAnimator> animator_;
};
