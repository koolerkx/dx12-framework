#include <imgui.h>

#include <algorithm>
#include <cstdio>
#include <filesystem>

#include "Game/Serialization/scene_serializer.h"
#include "Game/game_object.h"
#include "Game/scene.h"
#include "Game/scene_key.h"
#include "Game/scene_manager.h"
#include "Graphic/graphic.h"
#include "editor_layer.h"

void EditorLayer::DrawSceneMenu() {
  if (ImGui::BeginMenu("Scene")) {
    if (ImGui::MenuItem("New Scene")) {
      if (scene_ && scene_->GetContext()) {
        scene_->GetContext()->GetSceneManager()->RequestLoad(DefaultScenes::EMPTY);
      }
    }
    if (ImGui::MenuItem("Load Scene")) {
      ScanSceneFiles();
      selected_scene_index_ = -1;
      show_load_scene_modal_ = true;
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Save Scene")) {
      if (scene_) {
        strncpy_s(scene_name_buffer_, scene_->GetSceneName().c_str(), sizeof(scene_name_buffer_) - 1);
      }
      save_and_dump_ = false;
      show_save_scene_modal_ = true;
    }
    if (ImGui::MenuItem("Dump Settings")) {
      if (scene_) {
        strncpy_s(scene_name_buffer_, scene_->GetSceneName().c_str(), sizeof(scene_name_buffer_) - 1);
      }
      show_dump_setting_modal_ = true;
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Save + Dump")) {
      if (scene_) {
        strncpy_s(scene_name_buffer_, scene_->GetSceneName().c_str(), sizeof(scene_name_buffer_) - 1);
      }
      save_and_dump_ = true;
      show_save_scene_modal_ = true;
    }

    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Scene")) {
    for (const auto& [key, name] : SceneKeyTable::Instance().GetAll()) {
      if (ImGui::MenuItem(name.data())) {
        scene_->GetContext()->GetSceneManager()->RequestLoad(key);
      }
    }
    ImGui::EndMenu();
  }
}

void EditorLayer::DrawSaveSceneModal() {
  if (show_save_scene_modal_) {
    ImGui::OpenPopup(save_and_dump_ ? "Save + Dump##modal" : "Save Scene##modal");
    show_save_scene_modal_ = false;
    save_excluded_objects_.clear();
  }

  const char* popup_id = save_and_dump_ ? "Save + Dump##modal" : "Save Scene##modal";
  if (ImGui::BeginPopupModal(popup_id, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Scene Name:");
    ImGui::InputText("##scene_name", scene_name_buffer_, sizeof(scene_name_buffer_));

    ImGui::Separator();

    ImGui::Text("Include GameObjects:");
    ImGui::SameLine();
    if (ImGui::SmallButton("All")) {
      save_excluded_objects_.clear();
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("None")) {
      if (scene_) {
        for (const auto& go : scene_->GetGameObjects()) {
          if (go && !go->IsTransient()) save_excluded_objects_.insert(go.get());
        }
      }
    }

    ImGui::BeginChild("##go_tree", ImVec2(300, 200), ImGuiChildFlags_Borders);
    if (scene_) {
      for (const auto& go : scene_->GetGameObjects()) {
        if (!go || go->GetParent() != nullptr || go->IsTransient()) continue;
        DrawSaveExclusionTree(go.get(), false);
      }
    }
    ImGui::EndChild();

    ImGui::Separator();

    if (ImGui::Button("Confirm", ImVec2(120, 0))) {
      if (scene_ && scene_name_buffer_[0] != '\0') {
        std::string name(scene_name_buffer_);
        scene_->SetSceneName(name);
        bool ok = save_and_dump_ ? SceneSerializer::SaveAndDump(*scene_, name, save_excluded_objects_)
                                 : SceneSerializer::SaveScene(*scene_, name, save_excluded_objects_);
        save_status_success_ = ok;
        save_status_timer_ = 3.0f;
      }
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

void EditorLayer::DrawSaveExclusionTree(GameObject* go, bool parent_excluded) {
  if (!go) return;

  bool self_excluded = save_excluded_objects_.contains(go);
  bool effectively_excluded = parent_excluded || self_excluded;

  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
  if (go->GetChildren().empty()) flags |= ImGuiTreeNodeFlags_Leaf;

  if (parent_excluded) {
    bool disabled_check = false;
    ImGui::BeginDisabled();
    ImGui::Checkbox(("##excl_" + go->GetName()).c_str(), &disabled_check);
    ImGui::EndDisabled();
  } else {
    bool included = !self_excluded;
    if (ImGui::Checkbox(("##excl_" + go->GetName()).c_str(), &included)) {
      if (included)
        save_excluded_objects_.erase(go);
      else
        save_excluded_objects_.insert(go);
    }
  }

  ImGui::SameLine();

  if (effectively_excluded) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
  bool open = ImGui::TreeNodeEx(static_cast<void*>(go), flags, "%s", go->GetName().c_str());
  if (effectively_excluded) ImGui::PopStyleColor();

  if (open) {
    for (auto* child : go->GetChildren()) {
      if (child && !child->IsTransient()) DrawSaveExclusionTree(child, effectively_excluded);
    }
    ImGui::TreePop();
  }
}

void EditorLayer::DrawDumpSettingModal() {
  if (show_dump_setting_modal_) {
    ImGui::OpenPopup("Dump Settings##modal");
    show_dump_setting_modal_ = false;
  }

  if (ImGui::BeginPopupModal("Dump Settings##modal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Scene Name:");
    ImGui::InputText("##settings_name", scene_name_buffer_, sizeof(scene_name_buffer_));

    if (ImGui::Button("Confirm", ImVec2(120, 0))) {
      if (scene_ && scene_name_buffer_[0] != '\0') {
        std::string name(scene_name_buffer_);
        bool ok = SceneSerializer::DumpSettings(*scene_, name);
        save_status_success_ = ok;
        save_status_timer_ = 3.0f;
      }
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

void EditorLayer::ScanSceneFiles() {
  scene_file_list_.clear();
  auto scenes_dir = SceneSerializer::GetScenesDirectory();
  if (!std::filesystem::exists(scenes_dir)) return;
  for (auto& entry : std::filesystem::directory_iterator(scenes_dir)) {
    auto filename = entry.path().filename().string();
    if (filename.size() > 11 && filename.substr(filename.size() - 11) == ".scene.yaml") {
      scene_file_list_.push_back(filename.substr(0, filename.size() - 11));
    }
  }
  std::sort(scene_file_list_.begin(), scene_file_list_.end());
}

void EditorLayer::DrawLoadSceneModal() {
  if (show_load_scene_modal_) {
    ImGui::OpenPopup("Load Scene##modal");
    show_load_scene_modal_ = false;
  }

  if (ImGui::BeginPopupModal("Load Scene##modal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Select a scene file:");

    ImVec2 list_size(300, 200);
    if (ImGui::BeginChild("##scene_list", list_size, ImGuiChildFlags_Borders)) {
      for (int i = 0; i < static_cast<int>(scene_file_list_.size()); ++i) {
        bool selected = (i == selected_scene_index_);
        if (ImGui::Selectable(scene_file_list_[i].c_str(), selected)) {
          selected_scene_index_ = i;
        }
      }
    }
    ImGui::EndChild();

    if (scene_file_list_.empty()) {
      ImGui::TextDisabled("No scene files found in Content/scenes/");
    }

    ImGui::Separator();
    ImGui::Text("Load Scope:");

    static int scope_radio = 0;
    ImGui::RadioButton("Both", &scope_radio, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Scene Only", &scope_radio, 1);
    ImGui::SameLine();
    ImGui::RadioButton("Settings Only", &scope_radio, 2);

    static int load_mode_radio = 0;
    bool scope_has_scene = scope_radio != 2;
    if (!scope_has_scene) load_mode_radio = 0;

    if (scope_has_scene) {
      ImGui::Separator();
      ImGui::Text("Load Mode:");
      ImGui::RadioButton("New Scene", &load_mode_radio, 0);
      ImGui::SameLine();
      ImGui::RadioButton("Add to Current", &load_mode_radio, 1);
    }

    ImGui::Separator();

    bool can_confirm = selected_scene_index_ >= 0 && selected_scene_index_ < static_cast<int>(scene_file_list_.size());
    ImGui::BeginDisabled(!can_confirm);
    if (ImGui::Button("Confirm", ImVec2(120, 0))) {
      pending_load_path_ = scene_file_list_[selected_scene_index_];
      pending_load_scope_ = static_cast<LoadScope>(scope_radio);
      pending_load_additive_ = load_mode_radio == 1;

      bool load_into_current = pending_load_scope_ == LoadScope::SettingsOnly || pending_load_additive_;
      if (load_into_current) {
        if (scene_) {
          save_status_success_ = SceneSerializer::Load(*scene_, pending_load_path_, pending_load_scope_);
          if (pending_load_scope_ != LoadScope::SceneOnly) ApplyLoadedShadowSettings();
        } else {
          save_status_success_ = false;
        }
        save_status_timer_ = 3.0f;
        pending_load_path_.clear();
      } else if (scene_ && scene_->GetContext()) {
        scene_->GetContext()->GetSceneManager()->RequestLoad(DefaultScenes::BLANK);
      }

      ImGui::CloseCurrentPopup();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

void EditorLayer::ApplyLoadedShadowSettings() {
  if (!save_status_success_ || !scene_) return;
  auto& shadow = scene_->GetShadowSetting();
  pending_cascade_count_ = shadow.GetCascadeCount();
  pending_shadow_resolution_ = shadow.GetResolution();
}

void EditorLayer::DrawSceneSettings() {
  ImGui::Begin("Scene Settings");

  if (!scene_) {
    ImGui::TextDisabled("No scene loaded");
    ImGui::End();
    return;
  }

  auto& bg = scene_->GetBackgroundSetting();

  if (ImGui::CollapsingHeader("Background", ImGuiTreeNodeFlags_DefaultOpen)) {
    static const char* kModeNames[] = {"Clear Color", "Skybox"};
    int mode = static_cast<int>(bg.GetMode());
    if (ImGui::Combo("Mode", &mode, kModeNames, IM_ARRAYSIZE(kModeNames))) {
      bg.SetMode(static_cast<BackgroundMode>(mode));
    }

    if (bg.GetMode() == BackgroundMode::ClearColor) {
      Color color = bg.GetClearColor();
      if (ImGui::ColorEdit4("Clear Color", &color.x)) {
        bg.SetClearColorValue(color);
      }
    } else {
      ImGui::Text("Skybox: %s", bg.HasSkybox() ? "Loaded" : "None");
    }
  }

  auto& light = scene_->GetLightSetting();

  if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
    float azimuth = light.GetAzimuth();
    if (ImGui::SliderFloat("Azimuth", &azimuth, -180.0f, 180.0f, "%.1f deg")) {
      light.SetAzimuth(azimuth);
    }

    float elevation = light.GetElevation();
    if (ImGui::SliderFloat("Elevation", &elevation, -90.0f, 90.0f, "%.1f deg")) {
      light.SetElevation(elevation);
    }

    float intensity = light.GetIntensity();
    if (ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, 5.0f)) {
      light.SetIntensity(intensity);
    }

    Math::Vector3 dir_color = light.GetDirectionalColor();
    if (ImGui::ColorEdit3("Directional Color", &dir_color.x)) {
      light.SetDirectionalColor(dir_color);
    }

    float ambient_intensity = light.GetAmbientIntensity();
    if (ImGui::DragFloat("Ambient Intensity", &ambient_intensity, 0.01f, 0.0f, 2.0f)) {
      light.SetAmbientIntensity(ambient_intensity);
    }

    Math::Vector3 ambient_color = light.GetAmbientColor();
    if (ImGui::ColorEdit3("Ambient Color", &ambient_color.x)) {
      light.SetAmbientColor(ambient_color);
    }
  }

  auto& shadow = scene_->GetShadowSetting();

  if (ImGui::CollapsingHeader("Shadow", ImGuiTreeNodeFlags_DefaultOpen)) {
    bool enabled = shadow.IsEnabled();
    if (ImGui::Checkbox("Enabled", &enabled)) {
      shadow.SetEnabled(enabled);
    }

    static constexpr const char* CASCADE_LABELS[] = {"1", "2", "3", "4"};
    int cascade_count = static_cast<int>(shadow.GetCascadeCount()) - 1;
    if (ImGui::Combo("Cascade Count", &cascade_count, CASCADE_LABELS, IM_ARRAYSIZE(CASCADE_LABELS))) {
      pending_cascade_count_ = static_cast<uint32_t>(cascade_count) + 1;
    }

    static constexpr uint32_t RESOLUTION_OPTIONS[] = {512, 1024, 2048, 4096};
    static constexpr const char* RESOLUTION_LABELS[] = {"512", "1024", "2048", "4096"};
    uint32_t current_res = shadow.GetResolution();
    int selected = 2;
    for (int i = 0; i < 4; ++i) {
      if (RESOLUTION_OPTIONS[i] == current_res) {
        selected = i;
        break;
      }
    }
    if (ImGui::Combo("Resolution", &selected, RESOLUTION_LABELS, IM_ARRAYSIZE(RESOLUTION_LABELS))) {
      shadow.SetResolution(RESOLUTION_OPTIONS[selected]);
      pending_shadow_resolution_ = RESOLUTION_OPTIONS[selected];
    }

    static constexpr const char* ALGORITHM_LABELS[] = {"Hard", "PCF 3x3", "Poisson Disk", "Rotated Poisson Disk", "PCSS"};
    int algo = static_cast<int>(shadow.GetAlgorithm());
    if (ImGui::Combo("Algorithm", &algo, ALGORITHM_LABELS, IM_ARRAYSIZE(ALGORITHM_LABELS))) {
      shadow.SetAlgorithm(static_cast<ShadowAlgorithm>(algo));
    }

    if (shadow.GetAlgorithm() == ShadowAlgorithm::PCSS) {
      float light_size = shadow.GetLightSize();
      if (ImGui::SliderFloat("Light Size", &light_size, 0.1f, 20.0f, "%.1f")) {
        shadow.SetLightSize(light_size);
      }
    }

    float distance = shadow.GetShadowDistance();
    if (ImGui::DragFloat("Shadow Distance", &distance, 1.0f, 1.0f, 1000.0f, "%.0f")) {
      shadow.SetShadowDistance(distance);
    }

    float light_dist = shadow.GetLightDistance();
    if (ImGui::DragFloat("Light Distance", &light_dist, 1.0f, 1.0f, 1000.0f, "%.0f")) {
      shadow.SetLightDistance(light_dist);
    }

    float blend_range = shadow.GetCascadeBlendRange();
    if (ImGui::DragFloat("Cascade Blend", &blend_range, 0.01f, 0.0f, 0.5f, "%.2f")) {
      shadow.SetCascadeBlendRange(blend_range);
    }

    auto& shadow_data = graphic_->GetShadowFrameData();
    if (shadow_data.cascade_count > 1) {
      char splits_buf[128];
      int offset = 0;
      for (uint32_t i = 0; i < shadow_data.cascade_count; ++i) {
        if (i > 0) offset += std::snprintf(splits_buf + offset, sizeof(splits_buf) - offset, " | ");
        offset += std::snprintf(splits_buf + offset, sizeof(splits_buf) - offset, "%.1f", shadow_data.cascade_split_distances[i]);
      }
      ImGui::Text("Cascades: %s", splits_buf);
    }

    for (uint32_t i = 0; i < shadow.GetCascadeCount(); ++i) {
      char label[32];
      std::snprintf(label, sizeof(label), "Cascade %u", i);
      if (ImGui::TreeNode(label)) {
        char depth_label[32];
        char normal_label[32];
        std::snprintf(depth_label, sizeof(depth_label), "Depth Bias##%u", i);
        std::snprintf(normal_label, sizeof(normal_label), "Normal Bias##%u", i);

        float depth_bias = shadow.GetCascadeDepthBias(i);
        if (ImGui::DragFloat(depth_label, &depth_bias, 0.0001f, 0.0f, 0.1f, "%.4f")) {
          shadow.SetCascadeDepthBias(i, depth_bias);
        }

        float normal_bias = shadow.GetCascadeNormalBias(i);
        if (ImGui::DragFloat(normal_label, &normal_bias, 0.001f, 0.0f, 0.5f, "%.3f")) {
          shadow.SetCascadeNormalBias(i, normal_bias);
        }

        ImGui::TreePop();
      }
    }

    auto shadow_color = shadow.GetShadowColor();
    float color[3] = {shadow_color.x, shadow_color.y, shadow_color.z};
    if (ImGui::ColorEdit3("Shadow Color", color)) {
      shadow.SetShadowColor(Math::Vector3(color[0], color[1], color[2]));
    }

    if (ImGui::Button("Show Shadow Map")) {
      show_shadow_map_ = true;
    }
  }

  ImGui::End();
}
