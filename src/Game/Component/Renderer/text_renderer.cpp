#include "text_renderer.h"

#include <DirectXMath.h>

#include "Component/billboard_helper.h"
#include "Component/pivot_type.h"
#include "Game/Asset/asset_manager.h"
#include "Graphic/Pipeline/shader_descriptors.h"
#include "game_context.h"
#include "Component/transform_component.h"

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

  DirectX::XMFLOAT3 worldPos = transform->GetWorldPosition();
  DirectX::XMFLOAT3 camPos = packet.main_camera.position;
  DirectX::XMVECTOR worldVec = XMLoadFloat3(&worldPos);
  DirectX::XMVECTOR camVec = XMLoadFloat3(&camPos);
  cmd.depth = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(DirectX::XMVectorSubtract(worldVec, camVec)));

  cmd.material_instance.material = cmd.material;
  cmd.material_instance.albedo_texture_index = texture->GetBindlessIndex();
  cmd.material_instance.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);

  cmd.instances.reserve(text_mesh_handle_.GetGlyphCount());

  DirectX::XMFLOAT2 text_size = {text_mesh_handle_.GetWidth(), text_mesh_handle_.GetHeight()};
  DirectX::XMFLOAT2 pivot_offset = pivot_.CalculateOffset(text_size);

  for (size_t i = 0; i < text_mesh_handle_.GetGlyphCount(); ++i) {
    const GlyphLayoutData* glyph = text_mesh_handle_.GetGlyph(i);
    if (!glyph || glyph->width <= 0.0f || glyph->height <= 0.0f) {
      continue;
    }

    SpriteInstanceData instance{};

    // Y-coordinate is negated for world space text (Y-up coordinate system)
    float glyph_x_relative = glyph->x - pivot_offset.x;
    float glyph_y_relative = glyph->y - pivot_offset.y;

    DirectX::XMVECTOR glyph_center = DirectX::XMVectorSet(glyph_x_relative + glyph->width * 0.5f,
      -(glyph_y_relative + glyph->height * 0.5f),  // Negate Y for world space
      0.01f,                                       // Small Z offset to avoid z-fighting
      0.0f);

    DirectX::XMMATRIX glyph_translation = DirectX::XMMatrixTranslationFromVector(glyph_center);
    DirectX::XMMATRIX size_scale = DirectX::XMMatrixScaling(glyph->width, glyph->height, 1.0f);

    DirectX::XMMATRIX base_world = CalculateBaseWorldMatrix(transform, packet.main_camera);
    DirectX::XMMATRIX world = size_scale * glyph_translation * base_world;
    DirectX::XMStoreFloat4x4(&instance.world_matrix, world);

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

DirectX::XMMATRIX TextRenderer::CalculateBaseWorldMatrix(TransformComponent* transform, const CameraData& camera) const {
  using namespace DirectX;
  XMMATRIX world = transform->GetWorldMatrix();

  if (billboard_mode_ == Billboard::Mode::None) {
    return world;
  }

  XMVECTOR scale, rotation, translation;
  XMMatrixDecompose(&scale, &rotation, &translation, world);

  XMFLOAT3 objPos;
  XMStoreFloat3(&objPos, translation);

  XMMATRIX billboardRot;
  if (billboard_mode_ == Billboard::Mode::Cylindrical) {
    billboardRot = Billboard::CreateCylindricalBillboardMatrix(objPos, camera.position);
  } else {
    billboardRot = Billboard::CreateSphericalBillboardMatrix(objPos, camera.position, camera.up);
  }

  // Fix text mirror issue: flip X axis for text with billboard
  // The UV system has negative Y scale which combines with billboard rotation to cause mirroring
  XMMATRIX flipX = XMMatrixScaling(-1.0f, 1.0f, 1.0f);

  XMMATRIX scaleMat = XMMatrixScalingFromVector(scale);
  XMMATRIX transMat = XMMatrixTranslationFromVector(translation);

  return scaleMat * flipX * billboardRot * transMat;
}

DirectX::XMMATRIX TextRenderer::GetBillboardWorldMatrix(const CameraData& camera) const {
  auto* transform = GetOwner()->GetTransform();
  if (!transform) return DirectX::XMMatrixIdentity();
  return CalculateBaseWorldMatrix(transform, camera);
}
