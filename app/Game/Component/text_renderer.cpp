#include "text_renderer.h"

#include "Game/Asset/asset_manager.h"
#include "game_context.h"
#include "transform_component.h"

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
  const Material* material = nullptr;

  switch (pass_tag_) {
    case RenderPassTag::Ui:
      // Use instanced material for UI text
      material = material_mgr.GetMaterial("Instanced_UI");
      break;
    case RenderPassTag::WorldOpaque:
      material = material_mgr.GetMaterial("Instanced_World_Text");
      break;
    case RenderPassTag::WorldTransparent:
      material = material_mgr.GetMaterial("Instanced_World_Text_Transparent");
      break;
    default:
      return;
  }

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
      DirectX::XMVECTOR glyph_center = DirectX::XMVectorSet(glyph->x + glyph->width * 0.5f, glyph->y + glyph->height * 0.5f, 0.0f, 0.0f);

      DirectX::XMMATRIX glyph_translation = DirectX::XMMatrixTranslationFromVector(glyph_center);
      DirectX::XMMATRIX size_scale = DirectX::XMMatrixScaling(glyph->width, glyph->height, 1.0f);

      // Combine: Scale -> Translate (local glyph space) -> Parent Transform (world space)
      DirectX::XMMATRIX world = size_scale * glyph_translation * transform->GetWorldMatrix();
      // Transpose to align with Constant Buffer convention
      // Note: Unlike CBV (which has column-major interpretation that cancels transpose),
      // Vertex Buffer passes raw bytes. Shader will transpose back to restore original matrix.
      DirectX::XMStoreFloat4x4(&instance.world_matrix, DirectX::XMMatrixTranspose(world));

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

    if (is_transparent) {
      // Use TransparentDrawCommand
      TransparentDrawCommand cmd;
      cmd.mesh = quad_mesh;
      cmd.material = material;
      cmd.depth = 0.0f;

      // Setup material instance (shared across all glyphs)
      cmd.material_instance.material = cmd.material;
      cmd.material_instance.albedo_texture_index = texture->GetBindlessIndex();

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
        DirectX::XMVECTOR glyph_center = DirectX::XMVectorSet(glyph->x + glyph->width * 0.5f,
          -(glyph->y + glyph->height * 0.5f),  // Negate Y for world space
          0.01f,                               // Small Z offset to avoid z-fighting
          0.0f);

        DirectX::XMMATRIX glyph_translation = DirectX::XMMatrixTranslationFromVector(glyph_center);
        DirectX::XMMATRIX size_scale = DirectX::XMMatrixScaling(glyph->width, glyph->height, 1.0f);

        // Combine: Scale -> Translate (local glyph space) -> Parent Transform (world space)
        DirectX::XMMATRIX world = size_scale * glyph_translation * transform->GetWorldMatrix();
        // Transpose to align with Constant Buffer convention
        // Note: Unlike CBV (which has column-major interpretation that cancels transpose),
        // Vertex Buffer passes raw bytes. Shader will transpose back to restore original matrix.
        DirectX::XMStoreFloat4x4(&instance.world_matrix, DirectX::XMMatrixTranspose(world));

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
        DirectX::XMVECTOR glyph_center = DirectX::XMVectorSet(glyph->x + glyph->width * 0.5f,
          -(glyph->y + glyph->height * 0.5f),  // Negate Y for world space
          0.01f,                               // Small Z offset to avoid z-fighting
          0.0f);

        DirectX::XMMATRIX glyph_translation = DirectX::XMMatrixTranslationFromVector(glyph_center);
        DirectX::XMMATRIX size_scale = DirectX::XMMatrixScaling(glyph->width, glyph->height, 1.0f);

        // Combine: Scale -> Translate (local glyph space) -> Parent Transform (world space)
        DirectX::XMMATRIX world = size_scale * glyph_translation * transform->GetWorldMatrix();
        // Transpose to align with Constant Buffer convention
        // Note: Unlike CBV (which has column-major interpretation that cancels transpose),
        // Vertex Buffer passes raw bytes. Shader will transpose back to restore original matrix.
        DirectX::XMStoreFloat4x4(&instance.world_matrix, DirectX::XMMatrixTranspose(world));

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
