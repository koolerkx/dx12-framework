#include "text_renderer.h"

#if ENABLE_EDITOR
#include "Framework/Editor/render_inspector.h"
#endif
#include "Component/pivot_type.h"
#include "Component/transform_component.h"
#include "Framework/Asset/asset_manager.h"
#include "Framework/Render/render_handles.h"
#include "Framework/Shader/default_shaders.h"
#include "game_context.h"

using Math::Matrix4;
using Math::Vector2;
using Math::Vector3;

void TextRenderer::SetRenderLayer(RenderLayer layer) {
  switch (layer) {
    case RenderLayer::Opaque:
      render_settings_ = Rendering::RenderSettings::Opaque();
      break;
    case RenderLayer::Transparent:
      render_settings_ = Rendering::RenderSettings::Transparent();
      break;
    default:
      return;
  }
  render_layer_ = layer;
  SetDoubleSided(true);
  SetSampler(Rendering::SamplerType::LinearWrap);
}

void TextRenderer::SetPivot(Pivot::Preset preset) {
  text_pivot_ = Pivot::ToTextNormalized(preset);
}

void TextRenderer::SetPivot(const Vector2& normalized_pivot) {
  text_pivot_ = normalized_pivot;
}

TextRenderer::EditorData TextRenderer::GetEditorData() const {
  return {text_,
    font_family_,
    pixel_size_,
    color_,
    h_align_,
    v_align_,
    line_spacing_,
    letter_spacing_,
    use_kerning_,
    billboard_mode_,
    text_pivot_,
    render_settings_,
    render_layer_,
    render_tags_};
}

void TextRenderer::ApplyEditorData(const EditorData& data) {
  SetText(data.text);
  SetFont(data.font_family);
  SetPixelSize(data.pixel_size);
  SetColor(data.color);
  SetHorizontalAlign(data.h_align);
  SetVerticalAlign(data.v_align);
  SetLineSpacing(data.line_spacing);
  SetLetterSpacing(data.letter_spacing);
  SetUseKerning(data.use_kerning);
  SetBillboardMode(data.billboard_mode);
  SetPivot(data.pivot);
  SetRenderLayer(data.render_layer);
  SetBlendMode(data.render_settings.blend_mode);
  SetSampler(data.render_settings.sampler_type);
  SetDepthTest(data.render_settings.depth_test);
  SetDepthWrite(data.render_settings.depth_write);
  SetDoubleSided(data.render_settings.double_sided);
  SetRenderTags(data.render_tags);
}

void TextRenderer::OnRender(FramePacket& packet) {
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
    desc.flags = (render_layer_ != RenderLayer::Transparent) ? flags::Combine(MaterialFlags::AlphaTest) : 0u;
    if (!material_handle_.IsValid()) {
      material_handle_ = rs->AllocateMaterial(desc);
    } else {
      rs->UpdateMaterial(material_handle_, desc);
    }
    material_dirty_ = false;
  }

  MeshHandle rect_handle = context->GetAssetManager().GetDefaultMeshHandle(DefaultMesh::Rect);
  if (!rect_handle.IsValid()) return;

  Vector3 worldPos = transform->GetWorldPosition();
  Vector3 camPos = packet.main_camera.position;

  std::vector<InstanceData> glyph_instances;
  glyph_instances.reserve(text_mesh_handle_.GetGlyphCount());

  Vector2 text_size = {text_mesh_handle_.GetWidth(), text_mesh_handle_.GetHeight()};
  Vector2 pivot_offset(text_pivot_.x * text_size.x, text_pivot_.y * text_size.y);

  Matrix4 base_world = CalculateBaseWorldMatrix(transform, packet.main_camera);

  for (size_t i = 0; i < text_mesh_handle_.GetGlyphCount(); ++i) {
    const GlyphLayoutData* glyph = text_mesh_handle_.GetGlyph(i);
    if (!glyph || glyph->width <= 0.0f || glyph->height <= 0.0f) continue;

    float glyph_x_relative = glyph->x - pivot_offset.x;
    float glyph_y_relative = glyph->y - pivot_offset.y;

    Vector3 glyph_center(glyph_x_relative + glyph->width * 0.5f, -(glyph_y_relative + glyph->height * 0.5f), 0.01f);
    Matrix4 glyph_translation = Matrix4::CreateTranslation(glyph_center);
    Matrix4 size_scale = Matrix4::CreateScale(Vector3(glyph->width, glyph->height, 1.0f));

    glyph_instances.push_back({
      .world = size_scale * glyph_translation * base_world,
      .color = color_,
      .uv_offset = glyph->uv_offset,
      .uv_scale = glyph->uv_scale,
      .overlay_color = {0, 0, 0, 0},
    });
  }

  if (!glyph_instances.empty()) {
    InstancedRenderRequest request;
    request.mesh = rect_handle;
    request.shader_id = Shaders::Sprite::ID;
    request.render_settings = render_settings_;
    request.material = material_handle_;
    request.depth = Vector3::DistanceSquared(worldPos, camPos);
    request.layer = render_layer_;
    request.tags = render_tags_;
    packet.DrawInstanced(std::move(request), glyph_instances);
  }
}

void TextRenderer::OnDestroy() {
  if (material_handle_.IsValid()) {
    auto* context = GetOwner()->GetContext();
    if (context && context->GetRenderService()) {
      context->GetRenderService()->FreeMaterial(material_handle_);
    }
    material_handle_ = MaterialHandle::Invalid();
  }
  RendererComponent::OnDestroy();
}

void TextRenderer::RebuildTextMesh(AssetManager& asset_manager) {
  Text::TextLayoutProps props;
  props.pixel_size = pixel_size_;
  props.line_spacing = line_spacing_;
  props.letter_spacing = letter_spacing_;
  props.h_align = h_align_;
  props.v_align = v_align_;
  props.use_kerning = use_kerning_;

  text_mesh_handle_ = asset_manager.CreateTextMesh(text_, font_family_, pixel_size_, props);
}

Matrix4 TextRenderer::CalculateBaseWorldMatrix(TransformComponent* transform, const CameraData& camera) const {
  Matrix4 world = transform->GetWorldMatrix();

  if (billboard_mode_ == Billboard::Mode::None) {
    return world;
  }

  Vector3 scale = world.GetScale();
  Vector3 translation = world.GetTranslation();

  Vector3 target = camera.position;
  if (billboard_mode_ == Billboard::Mode::Cylindrical) {
    target.y = translation.y;
  }
  Matrix4 billboardRot = Matrix4::FaceTo(translation, target, camera.up);

  Matrix4 flipX = Matrix4::CreateScale(Vector3(-1.0f, 1.0f, 1.0f));

  Matrix4 scaleMat = Matrix4::CreateScale(scale);
  Matrix4 transMat = Matrix4::CreateTranslation(translation);

  return scaleMat * flipX * billboardRot * transMat;
}

Matrix4 TextRenderer::GetBillboardWorldMatrix(const CameraData& camera) const {
  auto* transform = GetOwner()->GetTransform();
  if (!transform) return Matrix4::Identity;
  return CalculateBaseWorldMatrix(transform, camera);
}

#if ENABLE_EDITOR
void TextRenderer::OnInspectorGUI() {
  auto data = GetEditorData();

  inspector::TextPropertiesEditor(data.text, data.font_family, data.pixel_size, data.h_align, data.line_spacing, data.letter_spacing);
  inspector::ColorEditor("Color", data.color);
  inspector::BillboardEditor(data.billboard_mode);
  editor_ui::DragFloat2("Pivot", &data.pivot.x, 0.01f, 0.0f, 1.0f);
  inspector::RenderLayerEditor(data.render_layer);
  inspector::RenderSettingsEditor(data.render_settings, false);

  Vector2 size = GetSize();
  editor_ui::Text("Size: %.1f x %.1f", size.x, size.y);

  ApplyEditorData(data);
}
#endif
