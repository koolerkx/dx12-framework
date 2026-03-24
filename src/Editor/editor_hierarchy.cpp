#include <imgui.h>

#include <algorithm>
#include <filesystem>

#include "Framework/Shader/default_shaders.h"
#include "Game/Component/Renderer/mesh_renderer.h"
#include "Game/Component/model_component.h"
#include "Game/Component/point_light_component.h"
#include "Game/Component/transform_component.h"
#include "Game/game_context.h"
#include "Game/game_object.h"
#include "Game/scene.h"
#include "editor_layer.h"

void EditorLayer::DrawHierarchy() {
  ImGui::Begin("Hierarchy");

  if (scene_) {
    if (ImGui::SmallButton("+")) {
      ImGui::OpenPopup("##AddGameObject");
    }

    if (ImGui::BeginPopup("##AddGameObject")) {
      if (ImGui::BeginMenu("Add Mesh")) {
        if (ImGui::MenuItem("Basic Cube")) CreateMeshGameObject("Cube", DefaultMesh::Cube, Shaders::Basic3D::ID);
        if (ImGui::MenuItem("Basic Sphere")) CreateMeshGameObject("Sphere", DefaultMesh::Sphere, Shaders::Basic3D::ID);
        if (ImGui::MenuItem("Basic Plane")) CreateMeshGameObject("Plane", DefaultMesh::Plane, Shaders::Basic3D::ID);
        ImGui::Separator();
        if (ImGui::MenuItem("PBR Cube")) CreateMeshGameObject("Cube", DefaultMesh::Cube, Shaders::PBR::ID);
        if (ImGui::MenuItem("PBR Sphere")) CreateMeshGameObject("Sphere", DefaultMesh::Sphere, Shaders::PBR::ID);
        if (ImGui::MenuItem("PBR Plane")) CreateMeshGameObject("Plane", DefaultMesh::Plane, Shaders::PBR::ID);
        ImGui::EndMenu();
      }
      if (ImGui::MenuItem("Add Model...")) {
        ScanModelFiles();
        show_add_model_modal_ = true;
      }
      bool can_add = static_cast<uint32_t>(CountPointLightsInScene()) < MAX_POINT_LIGHTS;
      if (ImGui::MenuItem("Add Point Light", nullptr, false, can_add)) {
        auto* go = scene_->CreateGameObject("PointLight");
        go->AddComponent<PointLightComponent>(PointLightComponent::Props{});
        selected_object_ = go;
      }
      if (!can_add) {
        ImGui::SetItemTooltip("Max %d point lights reached", MAX_POINT_LIGHTS);
      }
      ImGui::EndPopup();
    }

    ImGui::Separator();

    for (const auto& go : scene_->GetGameObjects()) {
      if (!go || go->GetParent() != nullptr) continue;
      DrawGameObjectNode(go.get());
    }
  }

  ImGui::End();
}

void EditorLayer::DrawGameObjectNode(GameObject* go) {
  if (!go) return;

  bool inactive = !go->IsActive();
  if (inactive) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
  if (go->GetChildren().empty()) flags |= ImGuiTreeNodeFlags_Leaf;
  if (go == selected_object_) flags |= ImGuiTreeNodeFlags_Selected;

  bool open = ImGui::TreeNodeEx(static_cast<void*>(go), flags, "%s", go->GetName().c_str());

  if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) selected_object_ = go;

  if (ImGui::BeginPopupContextItem()) {
    if (ImGui::MenuItem("Delete")) {
      pending_delete_ = go;
    }
    ImGui::EndPopup();
  }

  if (open) {
    for (auto* child : go->GetChildren()) {
      DrawGameObjectNode(child);
    }
    ImGui::TreePop();
  }

  if (inactive) ImGui::PopStyleColor();
}

void EditorLayer::DrawInspector() {
  ImGui::Begin("Inspector");

  if (selected_object_ && selected_object_->IsPendingDestroy()) {
    selected_object_ = nullptr;
  }

  if (selected_object_) {
    bool active = selected_object_->IsActive();
    if (ImGui::Checkbox("##Active", &active)) selected_object_->SetActive(active);
    ImGui::SameLine();
    char name_buf[256];
    snprintf(name_buf, sizeof(name_buf), "%s", selected_object_->GetName().c_str());
    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputText("##Name", name_buf, sizeof(name_buf), ImGuiInputTextFlags_EnterReturnsTrue) ||
        (!ImGui::IsItemActive() && ImGui::IsItemDeactivatedAfterEdit())) {
      selected_object_->SetName(name_buf);
    }
    ImGui::TextDisabled("UUID: %s", selected_object_->GetUUID().ToString().c_str());
    ImGui::Separator();

    for (const auto& comp : selected_object_->GetComponents()) {
      if (!comp) continue;
      ImGui::PushID(comp.get());

      if (auto* transform = dynamic_cast<TransformComponent*>(comp.get())) {
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
          bool dbg = comp->IsDebugDrawEnabled();
          ImGui::BeginDisabled(!debug_draw_enabled_);
          if (ImGui::Checkbox("Debug Draw##comp", &dbg)) comp->SetDebugDrawEnabled(dbg);
          ImGui::EndDisabled();
          transform->OnInspectorGUI();
        }
      } else if (auto* point_light = dynamic_cast<PointLightComponent*>(comp.get())) {
        if (ImGui::CollapsingHeader("PointLightComponent", ImGuiTreeNodeFlags_DefaultOpen)) {
          bool dbg = comp->IsDebugDrawEnabled();
          ImGui::BeginDisabled(!debug_draw_enabled_);
          if (ImGui::Checkbox("Debug Draw##comp", &dbg)) comp->SetDebugDrawEnabled(dbg);
          ImGui::EndDisabled();
          point_light->OnInspectorGUI();
        }
      } else if (ImGui::CollapsingHeader(comp->GetTypeName())) {
        comp->OnInspectorGUI();
      }

      ImGui::PopID();
    }
  } else {
    ImGui::TextDisabled("No object selected");
  }

  ImGui::End();
}

void EditorLayer::CreateMeshGameObject(const char* name, DefaultMesh mesh, ShaderId shader) {
  if (!scene_) return;
  auto* go = scene_->CreateGameObject(name);
  MeshRenderer::Props props;
  props.mesh_type = mesh;
  props.shader_id = shader;
  go->AddComponent<MeshRenderer>(props);
  selected_object_ = go;
}

void EditorLayer::ApplyPendingModelCreation() {
  if (pending_model_path_.empty() || !scene_ || !scene_->GetContext()) return;
  std::string path = "Content/models/" + pending_model_path_;
  auto model_data = scene_->GetContext()->GetAssetManager().LoadModel(path);
  if (model_data) {
    auto* go = scene_->CreateGameObject(pending_model_path_);
    go->AddComponent<ModelComponent>(ModelComponent::Props{
      .model = model_data,
      .shader_id = Shaders::PBR::ID,
    });
    selected_object_ = go;
  }
  pending_model_path_.clear();
}

void EditorLayer::DrawAddModelModal() {
  if (show_add_model_modal_) {
    ImGui::OpenPopup("Add Model##modal");
    show_add_model_modal_ = false;
  }

  if (ImGui::BeginPopupModal("Add Model##modal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Select a model:");
    if (ImGui::BeginChild("##model_list", ImVec2(300, 200), ImGuiChildFlags_Borders)) {
      for (int i = 0; i < static_cast<int>(model_file_list_.size()); ++i) {
        if (ImGui::Selectable(model_file_list_[i].c_str(), i == selected_model_index_)) selected_model_index_ = i;
      }
    }
    ImGui::EndChild();

    if (model_file_list_.empty()) {
      ImGui::TextDisabled("No model files found in Content/models/");
    }

    ImGui::Separator();

    bool has_selection = selected_model_index_ >= 0 && selected_model_index_ < static_cast<int>(model_file_list_.size());
    ImGui::BeginDisabled(!has_selection);
    if (ImGui::Button("Add", ImVec2(120, 0))) {
      pending_model_path_ = model_file_list_[selected_model_index_];
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

void EditorLayer::ScanModelFiles() {
  model_file_list_.clear();
  selected_model_index_ = -1;
  std::filesystem::path dir("Content/models");
  if (!std::filesystem::exists(dir)) return;
  for (auto& entry : std::filesystem::directory_iterator(dir)) {
    auto ext = entry.path().extension().string();
    if (ext == ".fbx" || ext == ".obj" || ext == ".gltf") {
      model_file_list_.push_back(entry.path().filename().string());
    }
  }
  std::sort(model_file_list_.begin(), model_file_list_.end());
}

int EditorLayer::CountPointLightsInScene() const {
  if (!scene_) return 0;
  int count = 0;
  for (const auto& go : scene_->GetGameObjects()) {
    if (go && !go->IsPendingDestroy() && go->GetComponent<PointLightComponent>()) ++count;
  }
  return count;
}
