#include <imgui.h>

#include "Game/Component/transform_component.h"
#include "Game/Debug/debug_drawer.h"
#include "Game/game.h"
#include "Graphic/Resource/Mesh/mesh_buffer_pool.h"
#include "Graphic/graphic.h"
#include "editor_layer.h"

void EditorLayer::DrawPlayControls() {
  auto state = game_->GetPlayState();

  ImGui::BeginDisabled(state == PlayState::Playing);
  if (ImGui::Button("Play")) game_->Play();
  ImGui::EndDisabled();

  ImGui::SameLine();
  ImGui::BeginDisabled(state != PlayState::Playing);
  if (ImGui::Button("Pause")) game_->Pause();
  ImGui::EndDisabled();

  ImGui::SameLine();
  ImGui::BeginDisabled(state == PlayState::Stopped);
  if (ImGui::Button("Stop")) game_->Stop();
  ImGui::EndDisabled();

  ImGui::SameLine();
  static constexpr const char* PLAY_STATE_LABELS[] = {"Stopped", "Playing", "Paused"};
  ImGui::TextDisabled("(%s)", PLAY_STATE_LABELS[static_cast<int>(state)]);
}

void EditorLayer::DrawDebugPanel() {
  ImGui::Begin("Debug");

  if (game_) {
    DrawPlayControls();
    ImGui::Separator();
  }

  auto& depth_cfg = graphic_->GetDepthViewConfig();
  auto& hdr_dbg = graphic_->GetHdrDebug();

  static const char* kDisplayViewNames[] = {"Normal", "Scene RT", "Depth Buffer"};
  int current = 0;
  if (hdr_dbg.debug_view) current = 1;
  if (depth_cfg.enabled) current = 2;

  bool vsync = graphic_->IsVSyncEnabled();
  if (ImGui::Checkbox("VSync", &vsync)) {
    graphic_->SetVSync(vsync);
  }

  if (ImGui::Checkbox("Wireframe", &wireframe_mode_)) {
    graphic_->SetWireframeMode(wireframe_mode_);
  }

  if (ImGui::Combo("Display View", &current, kDisplayViewNames, IM_ARRAYSIZE(kDisplayViewNames))) {
    hdr_dbg.debug_view = (current == 1);
    depth_cfg.enabled = (current == 2);
  }

  if (depth_cfg.enabled) {
    ImGui::DragFloat("Near Plane", &depth_cfg.near_plane, 0.01f, 0.001f, depth_cfg.far_plane);
    ImGui::DragFloat("Far Plane", &depth_cfg.far_plane, 1.0f, depth_cfg.near_plane, 100000.0f);
  }

  ImGui::Separator();

  if (debug_drawer_) {
    if (ImGui::Checkbox("Debug Draw", &debug_draw_enabled_)) {
      debug_drawer_->SetEnabled(debug_draw_enabled_);
    }
    ImGui::BeginDisabled(!debug_draw_enabled_);
    if (ImGui::SliderFloat("Debug Opacity", &debug_draw_opacity_, 0.0f, 1.0f)) {
      debug_drawer_->SetGlobalOpacity(debug_draw_opacity_);
    }
    ImGui::EndDisabled();
  }

  ImGui::Separator();

  if (ImGui::CollapsingHeader("Mesh Buffer Pool")) {
    auto stats = graphic_->GetMeshBufferPool().GetStats();

    float vertex_pct = stats.vertex_total > 0 ? static_cast<float>(stats.vertex_used) / static_cast<float>(stats.vertex_total) : 0.0f;
    float index_pct = stats.index_total > 0 ? static_cast<float>(stats.index_used) / static_cast<float>(stats.index_total) : 0.0f;

    char vertex_label[64];
    std::snprintf(vertex_label, sizeof(vertex_label), "Vertices: %u / %u", stats.vertex_used, stats.vertex_total);
    ImGui::ProgressBar(vertex_pct, ImVec2(-1, 0), vertex_label);

    char index_label[64];
    std::snprintf(index_label, sizeof(index_label), "Indices: %u / %u", stats.index_used, stats.index_total);
    ImGui::ProgressBar(index_pct, ImVec2(-1, 0), index_label);

    ImGui::Text("Meshes: %u / %u", stats.mesh_count, stats.max_mesh_count);
    ImGui::Text("Pending Frees: %u", stats.pending_frees);
  }

  ImGui::End();
}

void EditorLayer::DrawEditorSettings() {
  ImGui::Begin("Editor Settings", &show_editor_settings_);

  if (ImGui::CollapsingHeader("Transform Snap", ImGuiTreeNodeFlags_DefaultOpen)) {
    auto& snap = TransformComponent::GetSnapConfig();
    ImGui::DragFloat("Position Snap", &snap.position, 0.01f, 0.001f, 10.0f, "%.3f");
    ImGui::DragFloat("Rotation Snap", &snap.rotation, 0.1f, 0.01f, 45.0f, "%.2f");
    ImGui::DragFloat("Scale Snap", &snap.scale, 0.001f, 0.001f, 1.0f, "%.3f");
  }

  ImGui::End();
}

void EditorLayer::DrawPostFxPanel() {
  ImGui::Begin("Post FX", &show_postfx_);

  auto& bloom = graphic_->GetBloomConfig();

  if (ImGui::CollapsingHeader("Bloom", ImGuiTreeNodeFlags_DefaultOpen)) {
    bool prev_enabled = bloom.enabled;
    uint32_t prev_mip_levels = bloom.mip_levels;

    ImGui::Checkbox("Enabled##bloom", &bloom.enabled);
    int mip = static_cast<int>(bloom.mip_levels);
    if (ImGui::SliderInt("Mip Levels", &mip, 1, 8)) {
      bloom.mip_levels = static_cast<uint32_t>(mip);
    }
    ImGui::SliderFloat("Threshold", &bloom.threshold, 0.0f, 5.0f, "%.2f");
    ImGui::SliderFloat("Intensity", &bloom.intensity, 0.0f, 2.0f, "%.2f");

    if (bloom.enabled != prev_enabled || bloom.mip_levels != prev_mip_levels) {
      graphic_->RequestPipelineRebuild();
    }
  }

  auto& ssao = graphic_->GetSSAOConfig();

  if (ImGui::CollapsingHeader("SSAO", ImGuiTreeNodeFlags_DefaultOpen)) {
    bool prev_enabled = ssao.enabled;
    ImGui::Checkbox("Enabled##ssao", &ssao.enabled);
    ImGui::SliderFloat("Radius##ssao", &ssao.radius, 0.01f, 2.0f, "%.3f");
    ImGui::SliderFloat("Bias##ssao", &ssao.bias, 0.0f, 0.1f, "%.4f");
    ImGui::SliderFloat("Intensity##ssao", &ssao.intensity, 0.0f, 5.0f, "%.2f");
    int samples = static_cast<int>(ssao.sample_count);
    if (ImGui::SliderInt("Samples##ssao", &samples, 8, 32)) {
      ssao.sample_count = static_cast<uint32_t>(samples);
    }
    if (ssao.enabled != prev_enabled) {
      graphic_->RequestPipelineRebuild();
    }
  }

  auto& vignette = graphic_->GetVignetteConfig();

  if (ImGui::CollapsingHeader("Vignette", ImGuiTreeNodeFlags_DefaultOpen)) {
    bool prev_enabled = vignette.enabled;
    ImGui::Checkbox("Enabled##vignette", &vignette.enabled);
    ImGui::SliderFloat("Intensity##vignette", &vignette.intensity, 0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("Radius##vignette", &vignette.radius, 0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("Softness##vignette", &vignette.softness, 0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("Roundness##vignette", &vignette.roundness, 0.0f, 1.0f, "%.2f");
    ImGui::ColorEdit3("Color##vignette", vignette.vignette_color);
    if (vignette.enabled != prev_enabled) {
      graphic_->RequestPipelineRebuild();
    }
  }

  auto& fog = graphic_->GetFogConfig();

  if (ImGui::CollapsingHeader("Fog", ImGuiTreeNodeFlags_DefaultOpen)) {
    bool prev_enabled = fog.enabled;
    ImGui::Checkbox("Enabled##fog", &fog.enabled);
    ImGui::SliderFloat("Density##fog", &fog.density, 0.001f, 0.2f, "%.4f");
    ImGui::SliderFloat("Height Falloff##fog", &fog.height_falloff, 0.0f, 2.0f, "%.3f");
    ImGui::DragFloat("Base Height##fog", &fog.base_height, 0.1f, -100.0f, 100.0f, "%.1f");
    ImGui::DragFloat("Max Distance##fog", &fog.max_distance, 1.0f, 10.0f, 5000.0f, "%.0f");
    ImGui::ColorEdit3("Color##fog", fog.fog_color);
    if (fog.enabled != prev_enabled) {
      graphic_->RequestPipelineRebuild();
    }
  }

  auto& outline = graphic_->GetOutlineConfig();

  if (ImGui::CollapsingHeader("Outline", ImGuiTreeNodeFlags_DefaultOpen)) {
    bool prev_enabled = outline.enabled;
    ImGui::Checkbox("Enabled##outline", &outline.enabled);
    ImGui::SliderFloat("Depth Weight##outline", &outline.depth_weight, 0.0f, 5.0f, "%.2f");
    ImGui::SliderFloat("Normal Weight##outline", &outline.normal_weight, 0.0f, 5.0f, "%.2f");
    ImGui::SliderFloat("Edge Threshold##outline", &outline.edge_threshold, 0.0f, 1.0f, "%.3f");
    ImGui::SliderFloat("Depth Falloff##outline", &outline.depth_falloff, 0.0f, 1.0f, "%.3f");
    ImGui::SliderFloat("Thickness##outline", &outline.thickness, 0.5f, 5.0f, "%.1f");
    ImGui::ColorEdit3("Color##outline", outline.outline_color);
    if (outline.enabled != prev_enabled) {
      graphic_->RequestPipelineRebuild();
    }
  }

  ImGui::End();
}
