#if ENABLE_EDITOR
#include "mesh_renderer.h"

#include "Framework/Editor/editor_ui.h"
#include "Framework/Editor/render_inspector.h"
#include "Framework/Render/render_types.h"
#include "Framework/Shader/default_shaders.h"
#include "Framework/Utils/file_scanner.h"
#include "game_context.h"

namespace {

void DrawTexturePicker(MeshRenderer* renderer,
  const char* label,
  const std::string& current_path,
  TextureHandle current_tex,
  TextureSlot slot,
  const char* clear_id,
  const char* none_label = "(None)") {
  bool is_embedded = current_path.empty() && current_tex.IsValid();
  if (is_embedded) {
    editor_ui::Text("%s: (Embedded)", label);
    return;
  }

  std::string preview = current_path.empty() ? none_label : current_path;
  if (editor_ui::BeginCombo(label, preview.c_str())) {
    if (editor_ui::Selectable("(None)", current_path.empty())) {
      renderer->RequestTextureChange(slot, "");
    }
    auto files = file_utils::ScanDirectory("Content/textures", {".png", ".jpg", ".jpeg", ".tga", ".bmp"});
    for (const auto& tex : files) {
      std::string full_path = "Content/textures/" + tex;
      if (editor_ui::Selectable(tex.c_str(), current_path == full_path)) {
        renderer->RequestTextureChange(slot, full_path);
      }
    }
    editor_ui::EndCombo();
  }
  if (!current_path.empty()) {
    editor_ui::SameLine();
    if (editor_ui::SmallButton(clear_id)) {
      renderer->RequestTextureChange(slot, "");
    }
  }
}

}  // namespace

void MeshRenderer::OnInspectorGUI() {
  editor_ui::Text("Mesh: %s", HasMesh() ? "Loaded" : "None");

  DrawTexturePicker(this, "Texture", GetTexturePath(), GetTexture(), TextureSlot::Albedo, "X##clear_tex", "(None - White Fallback)");

  auto& shader_service = GetOwner()->GetContext()->GetRenderService()->GetShaderNameService();
  auto shader_name = shader_service.GetName(GetShaderId());
  editor_ui::Text("Shader: %.*s", static_cast<int>(shader_name.size()), shader_name.data());

  auto data = GetEditorData();

  inspector::ColorEditor("Color", data.color);
  inspector::RenderLayerEditor(data.render_layer);
  inspector::RenderSettingsEditor(data.render_settings, true);

  {
    struct TagEntry {
      const char* label;
      RenderTag tag;
    };
    static constexpr TagEntry TAG_ENTRIES[] = {
      {"Cast Shadow", RenderTag::CastShadow},
      {"Receive Shadow", RenderTag::ReceiveShadow},
      {"Lit", RenderTag::Lit},
    };

    std::string tag_preview;
    for (const auto& [label, tag] : TAG_ENTRIES) {
      if (HasTag(data.render_tags, tag)) {
        if (!tag_preview.empty()) tag_preview += ", ";
        tag_preview += label;
      }
    }
    if (tag_preview.empty()) tag_preview = "None";

    if (editor_ui::BeginCombo("Render Tags", tag_preview.c_str())) {
      for (const auto& [label, tag] : TAG_ENTRIES) {
        bool has = HasTag(data.render_tags, tag);
        if (editor_ui::Checkbox(label, &has)) {
          if (has)
            data.render_tags |= static_cast<uint32_t>(tag);
          else
            data.render_tags &= ~static_cast<uint32_t>(tag);
        }
      }
      editor_ui::EndCombo();
    }
  }

  editor_ui::SliderFloat("Specular Intensity", &data.specular_intensity, 0.0f, 2.0f);
  editor_ui::SliderFloat("Specular Power", &data.specular_power, 1.0f, 256.0f);

  editor_ui::SliderFloat("Rim Intensity", &data.rim_intensity, 0.0f, 2.0f);
  editor_ui::SliderFloat("Rim Power", &data.rim_power, 0.5f, 16.0f);
  editor_ui::ColorEdit3("Rim Color", &data.rim_color.x);
  editor_ui::Checkbox("Rim Shadow Affected", &data.rim_shadow_affected);

  bool is_pbr = (GetShaderId() == Shaders::PBR::ID);

  if (is_pbr) {
    editor_ui::SeparatorText("PBR");

    bool has_mr_map = GetMetallicRoughnessTexture().IsValid();
    editor_ui::SliderFloat("Metallic", &data.metallic, 0.0f, 1.0f);
    if (has_mr_map && editor_ui::IsItemHovered()) {
      editor_ui::SetTooltip("Multiplied with metallic-roughness map");
    }
    editor_ui::SliderFloat("Roughness", &data.roughness, 0.0f, 1.0f);
    if (has_mr_map && editor_ui::IsItemHovered()) {
      editor_ui::SetTooltip("Multiplied with metallic-roughness map");
    }

    bool has_emissive_map = GetEmissiveTexture().IsValid();
    editor_ui::ColorEdit3("Emissive", &data.emissive_color.x);
    if (has_emissive_map && editor_ui::IsItemHovered()) {
      editor_ui::SetTooltip("Multiplied with emissive map");
    }
    editor_ui::SliderFloat("Emissive Intensity", &data.emissive_intensity, 0.0f, 10.0f);

    editor_ui::SeparatorText("PBR Textures");

    DrawTexturePicker(this, "Normal Map", GetNormalTexturePath(), GetNormalTexture(), TextureSlot::Normal, "X##clear_normal");
    DrawTexturePicker(
      this, "MR Map", GetMetallicRoughnessPath(), GetMetallicRoughnessTexture(), TextureSlot::MetallicRoughness, "X##clear_mr");
    DrawTexturePicker(this, "Emissive Map", GetEmissivePath(), GetEmissiveTexture(), TextureSlot::Emissive, "X##clear_emissive");
  }

  ApplyEditorData(data);
  ResolvePendingTextures();
}

#endif
