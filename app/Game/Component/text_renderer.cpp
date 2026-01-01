#include "text_renderer.h"

#include "Component/billboard_helper.h"
#include "Component/pivot_type.h"
#include "Game/Asset/asset_manager.h"
#include "game_context.h"
#include "transform_component.h"

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

  // Rebuild text mesh if dirty
  if (dirty_) {
    RebuildTextMesh(context->GetAssetManager());
    dirty_ = false;
  }

  // Skip if no glyphs
  if (text_mesh_handle_.GetGlyphCount() == 0) {
    return;
  }

  // Get material from Graphic
  auto& material_mgr = context->GetGraphic()->GetMaterialManager();
  const Material* material = material_mgr.GetOrCreateMaterial(render_settings_);

  if (!material) return;

  auto* transform = GetOwner()->GetTransform();
  Texture* texture = text_mesh_handle_.GetTexture();
  if (!texture) return;

  // Get shared Quad mesh
  const Mesh* quad_mesh = context->GetAssetManager().GetDefaultMesh(DefaultMesh::Quad);
  if (!quad_mesh) return;

  // === UI PASS: Use Instanced Rendering (1 draw call per TextRenderer) ===
  if (pass_tag_ == RenderPassTag::Ui) {
    UiDrawCommand cmd;
    cmd.mesh = quad_mesh;
    cmd.material = material;
    cmd.layer_id = layer_id_;
    cmd.depth = static_cast<float>(layer_id_);

    // Setup material instance (shared across all glyphs)
    cmd.material_instance.material = cmd.material;
    cmd.material_instance.albedo_texture_index = texture->GetBindlessIndex();
    cmd.material_instance.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);

    // Reserve space for instances to optimize performance
    cmd.instances.reserve(text_mesh_handle_.GetGlyphCount());

    // Calculate pivot offset (Transform Position = Pivot)
    DirectX::XMFLOAT2 text_size = {text_mesh_handle_.GetWidth(), text_mesh_handle_.GetHeight()};
    DirectX::XMFLOAT2 pivot_offset = pivot_.CalculateOffset(text_size);

    // Collect all glyph instance data (CPU-side matrix calculation)
    for (size_t i = 0; i < text_mesh_handle_.GetGlyphCount(); ++i) {
      const GlyphLayoutData* glyph = text_mesh_handle_.GetGlyph(i);
      if (!glyph || glyph->width <= 0.0f || glyph->height <= 0.0f) {
        continue;  // Skip invisible glyphs
      }

      SpriteInstanceData instance{};

      // 1. Calculate per-glyph world matrix
      float glyph_x_relative = glyph->x - pivot_offset.x;
      float glyph_y_relative = glyph->y - pivot_offset.y;

      DirectX::XMVECTOR glyph_center =
        DirectX::XMVectorSet(glyph_x_relative + glyph->width * 0.5f, glyph_y_relative + glyph->height * 0.5f, 0.0f, 0.0f);

      DirectX::XMMATRIX glyph_translation = DirectX::XMMatrixTranslationFromVector(glyph_center);
      DirectX::XMMATRIX size_scale = DirectX::XMMatrixScaling(glyph->width, glyph->height, 1.0f);

      // Combine: Scale -> Translate (local glyph space, relative to pivot) -> Parent Transform (world space)
      DirectX::XMMATRIX world = size_scale * glyph_translation * transform->GetWorldMatrix();
      DirectX::XMStoreFloat4x4(&instance.world_matrix, world);

      // 2. Color (shared for all glyphs in this text)
      instance.color = color_;

      // 3. UV transform (handle Y-flip for UI coordinate system)
      // UI uses Y-down coord system, flip UV: offset' = offset + scale, scale' = -scale
      instance.uv_offset = DirectX::XMFLOAT2(glyph->uv_offset.x, glyph->uv_offset.y + glyph->uv_scale.y);
      instance.uv_scale = DirectX::XMFLOAT2(glyph->uv_scale.x, -glyph->uv_scale.y);

      cmd.instances.push_back(instance);
    }

    // Push single command containing all glyph instances (1 draw call!)
    if (!cmd.instances.empty()) {
      packet.ui_pass.push_back(cmd);
    }

  }
  // === WORLD PASS: Use Instanced Rendering (1 draw call per TextRenderer) ===
  else if (pass_tag_ == RenderPassTag::WorldOpaque || pass_tag_ == RenderPassTag::WorldTransparent) {
    // Determine which command type to use
    bool is_transparent = (pass_tag_ == RenderPassTag::WorldTransparent);

    // Calculate pivot offset (Transform Position = Pivot)
    // Note: For World Space, Y is negated (Y-up coordinate system)
    DirectX::XMFLOAT2 text_size = {text_mesh_handle_.GetWidth(), text_mesh_handle_.GetHeight()};
    DirectX::XMFLOAT2 pivot_offset = pivot_.CalculateOffset(text_size);

    if (is_transparent) {
      // Use TransparentDrawCommand
      TransparentDrawCommand cmd;
      cmd.mesh = quad_mesh;
      cmd.material = material;
      cmd.depth = 0.0f;

      // Setup material instance (shared across all glyphs)
      cmd.material_instance.material = cmd.material;
      cmd.material_instance.albedo_texture_index = texture->GetBindlessIndex();
      cmd.material_instance.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);

      // Reserve space for instances to optimize performance
      cmd.instances.reserve(text_mesh_handle_.GetGlyphCount());

      // Collect all glyph instance data (CPU-side matrix calculation)
      for (size_t i = 0; i < text_mesh_handle_.GetGlyphCount(); ++i) {
        const GlyphLayoutData* glyph = text_mesh_handle_.GetGlyph(i);
        if (!glyph || glyph->width <= 0.0f || glyph->height <= 0.0f) {
          continue;  // Skip invisible glyphs
        }

        SpriteInstanceData instance{};

        // 1. Calculate per-glyph world matrix
        // Note: Y-coordinate is negated for world space text (Y-up coordinate system)
        float glyph_x_relative = glyph->x - pivot_offset.x;
        float glyph_y_relative = glyph->y - pivot_offset.y;

        DirectX::XMVECTOR glyph_center = DirectX::XMVectorSet(glyph_x_relative + glyph->width * 0.5f,
          -(glyph_y_relative + glyph->height * 0.5f),  // Negate Y for world space
          0.01f,                                       // Small Z offset to avoid z-fighting
          0.0f);

        DirectX::XMMATRIX glyph_translation = DirectX::XMMatrixTranslationFromVector(glyph_center);
        DirectX::XMMATRIX size_scale = DirectX::XMMatrixScaling(glyph->width, glyph->height, 1.0f);

        // Combine: Scale -> Translate (local glyph space, relative to pivot) -> Parent Transform (world space)
        DirectX::XMMATRIX base_world = CalculateBaseWorldMatrix(transform, packet.main_camera);
        DirectX::XMMATRIX world = size_scale * glyph_translation * base_world;
        DirectX::XMStoreFloat4x4(&instance.world_matrix, world);

        // 2. Color (shared for all glyphs in this text)
        instance.color = color_;

        // 3. UV transform (no Y-flip for world text, uses standard texture coordinates)
        instance.uv_offset = glyph->uv_offset;
        instance.uv_scale = glyph->uv_scale;

        cmd.instances.push_back(instance);
      }

      // Push single command containing all glyph instances (1 draw call!)
      if (!cmd.instances.empty()) {
        packet.transparent_pass.push_back(cmd);
      }
    } else {
      // Use OpaqueDrawCommand
      OpaqueDrawCommand cmd;
      cmd.mesh = quad_mesh;
      cmd.material = material;
      cmd.depth = 0.0f;

      // Setup material instance (shared across all glyphs)
      cmd.material_instance.material = cmd.material;
      cmd.material_instance.albedo_texture_index = texture->GetBindlessIndex();
      cmd.material_instance.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);

      // Reserve space for instances to optimize performance
      cmd.instances.reserve(text_mesh_handle_.GetGlyphCount());

      // Collect all glyph instance data (CPU-side matrix calculation)
      for (size_t i = 0; i < text_mesh_handle_.GetGlyphCount(); ++i) {
        const GlyphLayoutData* glyph = text_mesh_handle_.GetGlyph(i);
        if (!glyph || glyph->width <= 0.0f || glyph->height <= 0.0f) {
          continue;  // Skip invisible glyphs
        }

        SpriteInstanceData instance{};

        // 1. Calculate per-glyph world matrix (CPU-side)
        // Note: Y-coordinate is negated for world space text (Y-up coordinate system)
        float glyph_x_relative = glyph->x - pivot_offset.x;
        float glyph_y_relative = glyph->y - pivot_offset.y;

        DirectX::XMVECTOR glyph_center = DirectX::XMVectorSet(glyph_x_relative + glyph->width * 0.5f,
          -(glyph_y_relative + glyph->height * 0.5f),  // Negate Y for world space
          0.01f,                                       // Small Z offset to avoid z-fighting
          0.0f);

        DirectX::XMMATRIX glyph_translation = DirectX::XMMatrixTranslationFromVector(glyph_center);
        DirectX::XMMATRIX size_scale = DirectX::XMMatrixScaling(glyph->width, glyph->height, 1.0f);

        // Combine: Scale -> Translate (local glyph space, relative to pivot) -> Parent Transform (world space)
        DirectX::XMMATRIX base_world = CalculateBaseWorldMatrix(transform, packet.main_camera);
        DirectX::XMMATRIX world = size_scale * glyph_translation * base_world;
        DirectX::XMStoreFloat4x4(&instance.world_matrix, world);

        // 2. Color (shared for all glyphs in this text)
        instance.color = color_;

        // 3. UV transform (no Y-flip for world text, uses standard texture coordinates)
        instance.uv_offset = glyph->uv_offset;
        instance.uv_scale = glyph->uv_scale;

        cmd.instances.push_back(instance);
      }

      // Push single command containing all glyph instances (1 draw call!)
      if (!cmd.instances.empty()) {
        packet.opaque_pass.push_back(cmd);
      }
    }
  }
}

void TextRenderer::RebuildTextMesh(AssetManager& asset_manager) {
  // Setup layout properties
  Text::TextLayoutProps props;
  props.pixel_size = pixel_size_;
  props.line_spacing = line_spacing_;
  props.letter_spacing = letter_spacing_;
  props.h_align = h_align_;
  props.v_align = v_align_;
  props.use_kerning = use_kerning_;

  // Create text mesh through AssetManager
  text_mesh_handle_ = asset_manager.CreateTextMesh(text_, font_family_, pixel_size_, props);
}

DirectX::XMMATRIX TextRenderer::CalculateBaseWorldMatrix(TransformComponent* transform, const CameraData& camera) const {
  using namespace DirectX;
  // Get the base world matrix from transform
  XMMATRIX world = transform->GetWorldMatrix();

  // If no billboard mode, return the original world matrix
  // Pivot is handled in OnRender
  if (billboard_mode_ == Billboard::Mode::None) {
    return world;
  }

  // Decompose the world matrix into TRS components
  XMVECTOR scale, rotation, translation;
  XMMatrixDecompose(&scale, &rotation, &translation, world);

  // Get object position for billboard calculation
  XMFLOAT3 objPos;
  XMStoreFloat3(&objPos, translation);

  // Calculate billboard rotation based on mode
  // (pivot is at origin, so this works correctly)
  XMMATRIX billboardRot;
  if (billboard_mode_ == Billboard::Mode::Cylindrical) {
    billboardRot = Billboard::CreateCylindricalBillboardMatrix(objPos, camera.position);
  } else {  // Spherical
    billboardRot = Billboard::CreateSphericalBillboardMatrix(objPos, camera.position, camera.up);
  }

  // Fix text mirror issue: flip X axis for text with billboard
  // The UV system has negative Y scale which combines with billboard rotation to cause mirroring
  XMMATRIX flipX = XMMatrixScaling(-1.0f, 1.0f, 1.0f);

  // Recompose matrix: Scale * FlipX * BillboardRotation * Translation
  // Note: We preserve position and scale from original transform, but replace rotation with billboard
  XMMATRIX scaleMat = XMMatrixScalingFromVector(scale);
  XMMATRIX transMat = XMMatrixTranslationFromVector(translation);

  return scaleMat * flipX * billboardRot * transMat;
}

DirectX::XMMATRIX TextRenderer::GetBillboardWorldMatrix(const CameraData& camera) const {
  auto* transform = GetOwner()->GetTransform();
  if (!transform) return DirectX::XMMatrixIdentity();
  return CalculateBaseWorldMatrix(transform, camera);
}
