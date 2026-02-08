#include "ui_text_renderer.h"

#include "Component/pivot_type.h"
#include "Component/transform_component.h"
#include "Game/Asset/asset_manager.h"
#include "Graphic/Pipeline/shader_descriptors.h"
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
  return {text_, font_family_, pixel_size_, color_, h_align_, v_align_,
          line_spacing_, letter_spacing_, use_kerning_, layer_id_,
          text_pivot_, render_settings_, render_tags_};
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
  }

  if (text_mesh_handle_.GetGlyphCount() == 0) {
    return;
  }

  auto& material_mgr = context->GetGraphic()->GetMaterialManager();

  const Material* material = material_mgr.GetOrCreateMaterial(Graphics::SpriteInstancedUIShader::ID, render_settings_);
  if (!material) return;

  auto* transform = GetOwner()->GetTransform();
  Texture* texture = text_mesh_handle_.GetTexture();
  if (!texture) return;

  const Mesh* quad_mesh = context->GetAssetManager().GetDefaultMesh(DefaultMesh::Quad);
  if (!quad_mesh) return;

  DrawCommand cmd;
  cmd.mesh = quad_mesh;
  cmd.material = material;
  cmd.depth = static_cast<float>(layer_id_);

  cmd.material_instance.material = cmd.material;
  cmd.material_instance.albedo_texture_index = texture->GetBindlessIndex();
  cmd.material_instance.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);

  cmd.instances.reserve(text_mesh_handle_.GetGlyphCount());

  Vector2 text_size = {text_mesh_handle_.GetWidth(), text_mesh_handle_.GetHeight()};
  Vector2 pivot_offset(text_pivot_.x * text_size.x, text_pivot_.y * text_size.y);

  for (size_t i = 0; i < text_mesh_handle_.GetGlyphCount(); ++i) {
    const GlyphLayoutData* glyph = text_mesh_handle_.GetGlyph(i);
    if (!glyph || glyph->width <= 0.0f || glyph->height <= 0.0f) {
      continue;
    }

    SpriteInstanceData instance{};

    float glyph_x_relative = glyph->x - pivot_offset.x;
    float glyph_y_relative = glyph->y - pivot_offset.y;

    Vector3 glyph_center(glyph_x_relative + glyph->width * 0.5f, glyph_y_relative + glyph->height * 0.5f, 0.0f);

    Matrix4 glyph_translation = Matrix4::CreateTranslation(glyph_center);
    Matrix4 size_scale = Matrix4::CreateScale(Vector3(glyph->width, glyph->height, 1.0f));

    Matrix4 world = size_scale * glyph_translation * transform->GetWorldMatrix();
    instance.world_matrix = world;

    instance.color = color_;

    instance.uv_offset = Vector2(glyph->uv_offset.x, glyph->uv_offset.y + glyph->uv_scale.y);
    instance.uv_scale = Vector2(glyph->uv_scale.x, -glyph->uv_scale.y);

    cmd.instances.push_back(instance);
  }

  if (!cmd.instances.empty()) {
    cmd.layer = RenderLayer::UI;
    cmd.tags = render_tags_;
    packet.AddCommand(std::move(cmd));
  }
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
