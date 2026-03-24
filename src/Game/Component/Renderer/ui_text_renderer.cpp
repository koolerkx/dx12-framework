#include "ui_text_renderer.h"

#if ENABLE_EDITOR
#include "Framework/Editor/render_inspector.h"
#endif
#include "Component/pivot_type.h"
#include "Component/transform_component.h"
#include "Framework/Shader/default_shaders.h"
#include "Framework/Render/render_handles.h"
#include "Framework/Asset/asset_manager.h"
#include "game_context.h"

using Math::Matrix4;
using Math::Vector2;
using Math::Vector3;

void UITextRenderer::SetPivot(Pivot::Preset preset) {
  text_pivot_ = Pivot::ToTextNormalized(preset);
}

void UITextRenderer::SetPivot(const Vector2& normalized_pivot) {
  text_pivot_ = normalized_pivot;
}

UITextRenderer::EditorData UITextRenderer::GetEditorData() const {
  return {text_,
    font_family_,
    pixel_size_,
    color_,
    h_align_,
    v_align_,
    line_spacing_,
    letter_spacing_,
    use_kerning_,
    layer_id_,
    text_pivot_,
    render_settings_,
    render_tags_};
}

void UITextRenderer::ApplyEditorData(const EditorData& data) {
  SetText(data.text);
  SetFont(data.font_family);
  SetPixelSize(data.pixel_size);
  SetColor(data.color);
  SetHorizontalAlign(data.h_align);
  SetVerticalAlign(data.v_align);
  SetLineSpacing(data.line_spacing);
  SetLetterSpacing(data.letter_spacing);
  SetUseKerning(data.use_kerning);
  SetLayerId(data.layer_id);
  SetPivot(data.pivot);
  SetBlendMode(data.render_settings.blend_mode);
  SetSampler(data.render_settings.sampler_type);
  SetRenderTags(data.render_tags);
}

void UITextRenderer::OnRender(FramePacket& packet) {
  if (text_.empty()) return;

  auto* context = GetOwner()->GetContext();
  if (!context) return;

  if (dirty_) {
    RebuildTextMesh(context->GetAssetManager());
    dirty_ = false;
    material_dirty_ = true;
  }

  if (text_mesh_handle_.GetGlyphCount() == 0) {
    return;
  }

  auto* rs = context->GetRenderService();
  auto* transform = GetOwner()->GetTransform();
  TextureHandle texture = text_mesh_handle_.GetTexture();
  if (!texture.IsValid()) return;

  if (!material_handle_.IsValid() || material_dirty_) {
    MaterialDescriptor desc{};
    desc.albedo_texture_index = texture.GetBindlessIndex();
    desc.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);
    desc.flags = flags::Combine(MaterialFlags::AlphaTest);
    if (!material_handle_.IsValid()) {
      material_handle_ = rs->AllocateMaterial(desc);
    } else {
      rs->UpdateMaterial(material_handle_, desc);
    }
    material_dirty_ = false;
  }

  MeshHandle rect_handle = context->GetAssetManager().GetDefaultMeshHandle(DefaultMesh::Rect);
  if (!rect_handle.IsValid()) return;

  std::vector<InstanceData> glyph_instances;
  glyph_instances.reserve(text_mesh_handle_.GetGlyphCount());

  Vector2 text_size = {text_mesh_handle_.GetWidth(), text_mesh_handle_.GetHeight()};
  Vector2 pivot_offset(text_pivot_.x * text_size.x, text_pivot_.y * text_size.y);

  for (size_t i = 0; i < text_mesh_handle_.GetGlyphCount(); ++i) {
    const GlyphLayoutData* glyph = text_mesh_handle_.GetGlyph(i);
    if (!glyph || glyph->width <= 0.0f || glyph->height <= 0.0f) continue;

    float glyph_x_relative = glyph->x - pivot_offset.x;
    float glyph_y_relative = glyph->y - pivot_offset.y;

    Vector3 glyph_center(glyph_x_relative + glyph->width * 0.5f, glyph_y_relative + glyph->height * 0.5f, 0.0f);
    Matrix4 glyph_translation = Matrix4::CreateTranslation(glyph_center);
    Matrix4 size_scale = Matrix4::CreateScale(Vector3(glyph->width, glyph->height, 1.0f));

    glyph_instances.push_back({
      .world = size_scale * glyph_translation * transform->GetWorldMatrix(),
      .color = color_,
      .uv_offset = Math::Vector2(glyph->uv_offset.x, glyph->uv_offset.y + glyph->uv_scale.y),
      .uv_scale = Math::Vector2(glyph->uv_scale.x, -glyph->uv_scale.y),
      .overlay_color = {0, 0, 0, 0},
    });
  }

  if (!glyph_instances.empty()) {
    InstancedRenderRequest request;
    request.mesh = rect_handle;
    request.shader_id = Shaders::Sprite::ID;
    request.render_settings = render_settings_;
    request.material = material_handle_;
    request.depth = static_cast<float>(layer_id_);
    request.layer = RenderLayer::UI;
    request.tags = render_tags_;
    packet.DrawInstanced(std::move(request), glyph_instances);
  }
}

void UITextRenderer::OnDestroy() {
  if (material_handle_.IsValid()) {
    auto* context = GetOwner()->GetContext();
    if (context && context->GetRenderService()) {
      context->GetRenderService()->FreeMaterial(material_handle_);
    }
    material_handle_ = MaterialHandle::Invalid();
  }
  RendererComponent::OnDestroy();
}

void UITextRenderer::RebuildTextMesh(AssetManager& asset_manager) {
  Text::TextLayoutProps props;
  props.pixel_size = pixel_size_;
  props.line_spacing = line_spacing_;
  props.letter_spacing = letter_spacing_;
  props.h_align = h_align_;
  props.v_align = v_align_;
  props.use_kerning = use_kerning_;

  text_mesh_handle_ = asset_manager.CreateTextMesh(text_, font_family_, pixel_size_, props);
}

#if ENABLE_EDITOR
void UITextRenderer::OnInspectorGUI() {
  auto data = GetEditorData();

  inspector::TextPropertiesEditor(data.text, data.font_family, data.pixel_size, data.h_align, data.line_spacing, data.letter_spacing);
  inspector::ColorEditor("Color", data.color);
  editor_ui::DragInt("Layer ID", &data.layer_id);
  editor_ui::DragFloat2("Pivot", &data.pivot.x, 0.01f, 0.0f, 1.0f);
  inspector::RenderSettingsEditor(data.render_settings, false);

  Vector2 size = GetSize();
  editor_ui::Text("Size: %.1f x %.1f", size.x, size.y);

  ApplyEditorData(data);
}
#endif
