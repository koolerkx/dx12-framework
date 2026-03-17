#pragma once
#include <optional>

#include "Asset/asset_handle.h"
#include "Component/pivot_type.h"
#include "Component/renderer_component.h"
#include "Component/sprite_sheet_animator.h"
#include "Framework/Math/Math.h"
#include "Framework/Render/render_handles.h"
#include "Framework/Render/render_settings.h"
#include "Framework/Serialize/serialize_node.h"
#include "Game/Asset/asset_manager.h"
#include "Graphic/Frame/frame_packet.h"
#include "Graphic/Resource/Texture/texture.h"
#include "game_context.h"
#include "game_object.h"


using Math::Vector2;
using Math::Vector4;

class UISpriteRenderer : public RendererComponent<UISpriteRenderer> {
 public:
  struct Props {
    std::string texture_path;
    Vector4 color = {1, 1, 1, 1};
    Vector2 size = {100, 100};
    std::optional<int> layer_id = std::nullopt;
    Vector2 pivot = {0.0f, 0.0f};
  };

  UISpriteRenderer(GameObject* owner) : RendererComponent(owner) {
  }

  UISpriteRenderer(GameObject* owner, const Props& props) : RendererComponent(owner) {
    if (!props.texture_path.empty()) SetTexturePath(props.texture_path);

    SetColor(props.color);
    SetSize(props.size);

    layer_id_ = props.layer_id.value_or(InheritParentUILayerId());
    SetPivot(props.pivot);
  }

  void SetTexture(Texture* tex) {
    texture_ = tex;
    texture_path_.clear();
    texture_handle_ = {};
    material_dirty_ = true;
  }

  void SetTexturePath(const std::string& path) {
    texture_path_ = path;
    if (path.empty()) {
      texture_ = nullptr;
      texture_handle_ = {};
      material_dirty_ = true;
      return;
    }
    auto* context = GetOwner()->GetContext();
    if (!context) return;
    texture_handle_ = context->GetAssetManager().LoadTexture(path);
    texture_ = texture_handle_.Get();
    material_dirty_ = true;
  }
  void SetColor(const Vector4& color) {
    color_ = color;
  }
  void SetSize(const Vector2& size) {
    size_ = size;
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

  std::optional<int> GetUILayerId() const override {
    return layer_id_;
  }

  void SetBlendMode(Rendering::BlendMode mode) {
    render_settings_.blend_mode = mode;
  }
  void SetSampler(Rendering::SamplerType type) {
    render_settings_.sampler_type = type;
    material_dirty_ = true;
  }
  const Rendering::RenderSettings& GetRenderSettings() const {
    return render_settings_;
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

  void SetPivot(Pivot::Preset preset);
  void SetPivot(const Vector2& normalized_pivot);
  const Vector2& GetPivot() const {
    return ui_pivot_;
  }

  void OnSerialize(framework::SerializeNode& node) const override {
    if (!texture_path_.empty()) {
      node.Write("Texture", texture_path_);
    }

    node.WriteVec4("Color", color_.x, color_.y, color_.z, color_.w);
    node.WriteVec2("Size", size_.x, size_.y);
    node.WriteVec2("Pivot", ui_pivot_.x, ui_pivot_.y);
    node.Write("LayerId", layer_id_);

    node.Write("BlendMode", static_cast<int>(render_settings_.blend_mode));
    node.Write("SamplerType", static_cast<int>(render_settings_.sampler_type));
    node.Write("RenderTags", static_cast<int>(render_tags_));

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
    node.ReadVec2("Pivot", ui_pivot_.x, ui_pivot_.y);
    SetLayerId(node.ReadInt("LayerId", layer_id_));

    render_settings_.blend_mode =
      static_cast<Rendering::BlendMode>(node.ReadInt("BlendMode", static_cast<int>(render_settings_.blend_mode)));
    render_settings_.sampler_type =
      static_cast<Rendering::SamplerType>(node.ReadInt("SamplerType", static_cast<int>(render_settings_.sampler_type)));
    render_tags_ = static_cast<RenderTagMask>(node.ReadInt("RenderTags", static_cast<int>(render_tags_)));

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
    int layer_id;
    Rendering::RenderSettings render_settings;
    RenderTagMask render_tags;
  };

  EditorData GetEditorData() const;
  void ApplyEditorData(const EditorData& data);

  SpriteSheetAnimator& GetAnimator();

  void OnUpdate(float dt) override;
  void OnRender(FramePacket& packet) override;
  void OnDestroy() override;

 private:
  Texture* texture_ = nullptr;
  std::string texture_path_;
  AssetHandle<Texture> texture_handle_;
  Vector4 color_ = {1, 1, 1, 1};
  Vector2 size_ = {100, 100};
  Vector2 uv_offset_ = {0.0f, 0.0f};
  Vector2 uv_scale_ = {1.0f, 1.0f};
  int layer_id_ = 0;

  MaterialHandle material_handle_;
  bool material_dirty_ = true;

  Rendering::RenderSettings render_settings_ = Rendering::RenderSettings::UI();
  Vector2 ui_pivot_ = {0.0f, 0.0f};
  RenderTagMask render_tags_ = 0;

  std::optional<SpriteSheetAnimator> animator_;
};
