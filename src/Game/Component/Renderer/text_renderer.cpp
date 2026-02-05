#include "text_renderer.h"

#include "Component/pivot_type.h"
#include "Component/transform_component.h"
#include "Game/Asset/asset_manager.h"
#include "Graphic/Pipeline/shader_descriptors.h"
#include "game_context.h"

using Math::Matrix4;
using Math::Vector2;
using Math::Vector3;

void TextRenderer::SetRenderLayer(RenderLayer layer) {
  switch (layer) {
    case RenderLayer::Opaque:
      render_settings_ = Rendering::RenderSettings::Opaque();
      shader_id_ = Graphics::SpriteInstancedWorldShader::ID;
      break;
    case RenderLayer::Transparent:
      render_settings_ = Rendering::RenderSettings::Transparent();
      shader_id_ = Graphics::SpriteInstancedWorldTransparentShader::ID;
      break;
    default:
      return;
  }
  render_layer_ = layer;
  SetDoubleSided(true);
  SetSampler(Rendering::SamplerType::LinearWrap);
}

void TextRenderer::SetPivot(Pivot::Preset preset) {
  pivot_.preset = preset;
}

void TextRenderer::SetPivot(const Pivot::Config& config) {
  pivot_ = config;
}

void TextRenderer::OnRender(FramePacket& packet) {
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
  const Material* material = material_mgr.GetOrCreateMaterial(shader_id_, render_settings_);
  if (!material) return;

  auto* transform = GetOwner()->GetTransform();
  Texture* texture = text_mesh_handle_.GetTexture();
  if (!texture) return;

  const Mesh* quad_mesh = context->GetAssetManager().GetDefaultMesh(DefaultMesh::Quad);
  if (!quad_mesh) return;

  DrawCommand cmd;
  cmd.mesh = quad_mesh;
  cmd.material = material;

  Vector3 worldPos = transform->GetWorldPosition();
  Vector3 camPos = packet.main_camera.position;
  cmd.depth = Vector3::DistanceSquared(worldPos, camPos);

  cmd.material_instance.material = cmd.material;
  cmd.material_instance.albedo_texture_index = texture->GetBindlessIndex();
  cmd.material_instance.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);

  cmd.instances.reserve(text_mesh_handle_.GetGlyphCount());

  Vector2 text_size = {text_mesh_handle_.GetWidth(), text_mesh_handle_.GetHeight()};
  Vector2 pivot_offset = pivot_.CalculateOffset(text_size);

  for (size_t i = 0; i < text_mesh_handle_.GetGlyphCount(); ++i) {
    const GlyphLayoutData* glyph = text_mesh_handle_.GetGlyph(i);
    if (!glyph || glyph->width <= 0.0f || glyph->height <= 0.0f) {
      continue;
    }

    SpriteInstanceData instance{};

    float glyph_x_relative = glyph->x - pivot_offset.x;
    float glyph_y_relative = glyph->y - pivot_offset.y;

    Vector3 glyph_center(glyph_x_relative + glyph->width * 0.5f, -(glyph_y_relative + glyph->height * 0.5f), 0.01f);

    Matrix4 glyph_translation = Matrix4::CreateTranslation(glyph_center);
    Matrix4 size_scale = Matrix4::CreateScale(Vector3(glyph->width, glyph->height, 1.0f));

    Matrix4 base_world = CalculateBaseWorldMatrix(transform, packet.main_camera);
    instance.world_matrix = size_scale * glyph_translation * base_world;

    instance.color = color_;

    instance.uv_offset = glyph->uv_offset;
    instance.uv_scale = glyph->uv_scale;

    cmd.instances.push_back(instance);
  }

  if (!cmd.instances.empty()) {
    cmd.layer = render_layer_;
    cmd.tags = render_tags_;
    packet.AddCommand(std::move(cmd));
  }
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
