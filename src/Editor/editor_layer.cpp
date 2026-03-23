#include "editor_layer.h"

#include <ImGuizmo.h>
#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>

#include <algorithm>
#include <cstdio>
#include <filesystem>

#include "Framework/Event/event_bus.hpp"
#include "Framework/Event/input_events.h"
#include "Framework/Input/input.h"
#include "Framework/Input/keyboard.h"
#include "Framework/Render/frame_packet.h"
#include "Framework/Shader/default_shaders.h"
#include "Game/Component/camera_component.h"
#include "Game/Component/model_component.h"
#include "Game/Component/point_light_component.h"
#include "Game/Component/transform_component.h"
#include "Game/Debug/debug_drawer.h"
#include "Game/Serialization/scene_serializer.h"
#include "Game/game.h"
#include "Game/game_context.h"
#include "Game/game_object.h"
#include "Game/scene.h"
#include "Game/scene_events.h"
#include "Game/scene_key.h"
#include "Game/scene_manager.h"
#include "Graphic/Descriptor/descriptor_heap_manager.h"
#include "Graphic/Pipeline/shader_registry.h"
#include "Graphic/Presentation/swapchain_manager.h"
#include "Graphic/Render/render_graph.h"
#include "Graphic/Resource/Mesh/mesh_buffer_pool.h"
#include "Graphic/graphic.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void EditorLayer::Initialize(HWND hwnd, Graphic& graphic) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  ImGui::StyleColorsDark();

  ImGui_ImplWin32_Init(hwnd);

  auto& heap_manager = graphic.GetDescriptorHeapManager();
  imgui_font_srv_ = heap_manager.GetSrvStaticAllocator().Allocate();

  ImGui_ImplDX12_InitInfo dx12_info = {};
  dx12_info.Device = graphic.GetDevice();
  dx12_info.CommandQueue = graphic.GetCommandQueue();
  dx12_info.NumFramesInFlight = Graphic::FRAME_BUFFER_COUNT;
  dx12_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
  dx12_info.SrvDescriptorHeap = heap_manager.GetGlobalSrvHeap();
  dx12_info.SrvDescriptorAllocFn =
    [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu) {
      auto* self = static_cast<EditorLayer*>(info->UserData);
      // Font SRV is pre-allocated
      *out_cpu = self->imgui_font_srv_.cpu;
      *out_gpu = self->imgui_font_srv_.gpu;
    };
  dx12_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE) {
    // Freed in Shutdown via imgui_font_srv_
  };
  dx12_info.UserData = this;
  ImGui_ImplDX12_Init(&dx12_info);

  graphic_ = &graphic;
  base_style_ = ImGui::GetStyle();
  UpdateScaling();
}

void EditorLayer::Shutdown(Graphic& graphic) {
  graphic.WaitForGpuIdle();
  ImGui_ImplDX12_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  if (imgui_font_srv_.IsValid()) {
    graphic.GetDescriptorHeapManager().GetSrvStaticAllocator().FreeImmediate(imgui_font_srv_.index, 1);
    imgui_font_srv_ = {};
  }
}

void EditorLayer::BeginFrame() {
  if (pending_delete_ && scene_) {
    if (selected_object_ == pending_delete_ || GameObject::IsDescendantOf(selected_object_, pending_delete_)) {
      selected_object_ = nullptr;
    }
    scene_->DestroyGameObject(pending_delete_);
    pending_delete_ = nullptr;
  }

  ApplyPendingModelCreation();

  if (pending_cascade_count_ > 0 && scene_) {
    graphic_->WaitForGpuIdle();
    graphic_->SetCascadeCount(pending_cascade_count_);
    scene_->GetShadowSetting().SetCascadeCount(pending_cascade_count_);
    pending_cascade_count_ = 0;
  }

  if (pending_shadow_resolution_ > 0) {
    graphic_->WaitForGpuIdle();
    graphic_->SetShadowMapResolution(pending_shadow_resolution_);
    pending_shadow_resolution_ = 0;
  }

  ImGui_ImplDX12_NewFrame();
  ImGui_ImplWin32_NewFrame();
  UpdateScaling();
  ImGui::NewFrame();
  ImGuizmo::BeginFrame();

  if (scene_ && scene_->GetContext() && scene_->GetContext()->GetInput()) {
    const auto& io = ImGui::GetIO();
    scene_->GetContext()->GetInput()->SetEnabled(!io.WantCaptureKeyboard);
  }
}

void EditorLayer::Render(ID3D12GraphicsCommandList* cmd) {
  if (imgui_visible_) {
    bool prev_pipeline = show_render_pipeline_;
    bool prev_shadow = show_shadow_map_;

    ClearBackbuffer(cmd);
    DrawDockSpace();
    DrawMainMenu();
    if (show_viewport_) DrawViewport(cmd);
    if (show_performance_) DrawFpsCounter();
    if (show_hierarchy_) DrawHierarchy();
    if (show_inspector_) DrawInspector();
    if (show_scene_settings_) DrawSceneSettings();
    if (show_debug_) DrawDebugPanel();
    if (show_editor_settings_) DrawEditorSettings();
    if (show_postfx_) DrawPostFxPanel();

    if (show_render_pipeline_) DrawRenderPipelinePanel(cmd);
    if (show_shadow_map_) DrawShadowMapPanel(cmd);

    if (show_render_pipeline_ != prev_pipeline) graphic_->SetPreviewPipelineActive(show_render_pipeline_);
    if (show_shadow_map_ != prev_shadow) graphic_->SetPreviewShadowActive(show_shadow_map_);
  } else if (input_system_) {
    input_system_->SetViewportTransform(0, 0, 1.0f, 1.0f);
  }

  ImGui::Render();
  ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmd);
}

void EditorLayer::SetDebugDrawer(DebugDrawer* drawer) {
  debug_drawer_ = drawer;
  if (drawer) {
    debug_draw_enabled_ = drawer->IsEnabled();
    debug_draw_opacity_ = drawer->GetGlobalOpacity();
  }
}

void EditorLayer::SetScene(IScene* scene) {
  if (scene_ == scene) return;
  scene_ = scene;
  selected_object_ = nullptr;
  pending_delete_ = nullptr;
  pending_model_path_.clear();
}

void EditorLayer::SubscribeEvents(EventBus& bus) {
  event_scope_.Subscribe<KeyDownEvent>(bus, [this](const KeyDownEvent& e) {
    if (e.key == Keyboard::KeyCode::F10) {
      imgui_visible_ = !imgui_visible_;
    }
  });

  event_scope_.Subscribe<SceneChangedEvent>(bus, [this](const SceneChangedEvent& e) {
    SetScene(e.new_scene);
    if (!pending_load_path_.empty() && e.new_scene) {
      save_status_success_ = SceneSerializer::Load(*e.new_scene, pending_load_path_, pending_load_scope_);
      ApplyLoadedShadowSettings();
      save_status_timer_ = 3.0f;
      pending_load_path_.clear();
    }
  });
}

bool EditorLayer::WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  // reserve for toggle imgui
  if (msg == WM_SYSKEYDOWN && wParam == VK_F10) return true;

  if (!imgui_visible_) return false;

  if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) return true;

  const ImGuiIO& io = ImGui::GetIO();
  if (io.WantCaptureMouse &&
      (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_MOUSEWHEEL || msg == WM_MOUSEMOVE))
    return true;
  if (io.WantCaptureKeyboard && (msg == WM_KEYDOWN || msg == WM_KEYUP || msg == WM_CHAR)) return true;

  return false;
}

void EditorLayer::DrawDockSpace() {
  ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
}

void EditorLayer::DrawMainMenu() {
  if (ImGui::BeginMainMenuBar()) {
    DrawSceneMenu();
    if (ImGui::BeginMenu("Window")) {
      ImGui::MenuItem("Performance", nullptr, &show_performance_);
      ImGui::MenuItem("Hierarchy", nullptr, &show_hierarchy_);
      ImGui::MenuItem("Inspector", nullptr, &show_inspector_);
      ImGui::MenuItem("Scene Settings", nullptr, &show_scene_settings_);
      ImGui::MenuItem("Debug", nullptr, &show_debug_);
      ImGui::MenuItem("Editor Settings", nullptr, &show_editor_settings_);
      ImGui::MenuItem("Post FX", nullptr, &show_postfx_);
      ImGui::MenuItem("Render Pipeline", nullptr, &show_render_pipeline_);
      ImGui::MenuItem("Viewport", nullptr, &show_viewport_);
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

  DrawSaveSceneModal();
  DrawDumpSettingModal();
  DrawLoadSceneModal();
  DrawAddModelModal();

  if (save_status_timer_ > 0.0f) {
    save_status_timer_ -= ImGui::GetIO().DeltaTime;
    float alpha = (std::min)(save_status_timer_, 1.0f);
    ImVec4 color = save_status_success_ ? ImVec4(0.2f, 0.8f, 0.2f, alpha) : ImVec4(0.9f, 0.2f, 0.2f, alpha);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, 40.0f * ui_scale_), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.7f * alpha);
    if (ImGui::Begin("##save_status",
          nullptr,
          ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav)) {
      ImGui::TextColored(color, "%s", save_status_success_ ? "Saved successfully" : "Save failed");
    }
    ImGui::End();
  }
}

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

void EditorLayer::ClearBackbuffer(ID3D12GraphicsCommandList* cmd) {
  auto& swapchain = graphic_->GetPresentationContext()->GetSwapChainManager();
  auto rtv = swapchain.GetCurrentRTV();
  cmd->ClearRenderTargetView(rtv, editor_bg_color_, 0, nullptr);
}

void EditorLayer::DrawViewport(ID3D12GraphicsCommandList* cmd) {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::Begin("Viewport", &show_viewport_);
  ImGui::PopStyleVar();

  auto* graph = graphic_->GetRenderGraph();
  auto handle = graphic_->GetPreviewHandles().viewport_rt;

  if (handle != RenderGraphHandle::Invalid) {
    graph->TransitionForRead(cmd, handle);
    auto gpu = graph->GetSrvGpuHandle(handle);
    ImVec2 avail = ImGui::GetContentRegionAvail();

    auto [rt_w, rt_h] = graph->GetTextureSize(handle);
    float rt_aspect = static_cast<float>(rt_w) / static_cast<float>(rt_h);
    float panel_aspect = avail.x / avail.y;
    float img_w, img_h;
    if (panel_aspect > rt_aspect) {
      img_h = avail.y;
      img_w = avail.y * rt_aspect;
    } else {
      img_w = avail.x;
      img_h = avail.x / rt_aspect;
    }

    float pad_x = (avail.x - img_w) * 0.5f;
    float pad_y = (avail.y - img_h) * 0.5f;
    ImVec2 cursor = ImGui::GetCursorPos();
    ImGui::SetCursorPos(ImVec2(cursor.x + pad_x, cursor.y + pad_y));
    ImGui::Image(static_cast<ImTextureID>(gpu.ptr), ImVec2(img_w, img_h));

    ImVec2 img_min = ImGui::GetItemRectMin();
    ImVec2 img_size = ImGui::GetItemRectSize();
    if (input_system_ && img_size.x > 0 && img_size.y > 0) {
      input_system_->SetViewportTransform(
        img_min.x, img_min.y, static_cast<float>(rt_w) / img_size.x, static_cast<float>(rt_h) / img_size.y);
    }

    DrawViewGizmo();
  }

  ImGui::End();
}

void EditorLayer::DrawViewGizmo() {
  if (!scene_) return;
  auto* camera = scene_->GetCameraSetting().GetActive();
  if (!camera) return;

  CameraData cam_data = camera->GetCameraData();

  // ImGuizmo uses RH convention (Z = eye-at, X = -X_lh).
  // Convert LH→RH by negating columns 0 and 2 of the view matrix.
  auto FlipHandedness = [](Math::Matrix4& m) {
    m._11 = -m._11;
    m._21 = -m._21;
    m._31 = -m._31;
    m._41 = -m._41;
    m._13 = -m._13;
    m._23 = -m._23;
    m._33 = -m._33;
    m._43 = -m._43;
  };

  FlipHandedness(cam_data.view);
  float* view = &cam_data.view._11;

  ImVec2 img_min = ImGui::GetItemRectMin();
  ImVec2 img_size = ImGui::GetItemRectSize();
  float gizmo_size = 128.0f;
  ImVec2 gizmo_pos(img_min.x + img_size.x - gizmo_size, img_min.y);

  ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
  ImGuizmo::SetOrthographic(camera->GetProjectionType() == ProjectionType::Orthographic);
  ImGuizmo::SetRect(img_min.x, img_min.y, img_size.x, img_size.y);
  constexpr float VIEW_GIZMO_DISTANCE = 8.0f;
  ImGuizmo::ViewManipulate(view, VIEW_GIZMO_DISTANCE, gizmo_pos, ImVec2(gizmo_size, gizmo_size), 0x10101010);

  if (ImGuizmo::IsUsingViewManipulate()) {
    FlipHandedness(cam_data.view);
    Math::Matrix4 world = cam_data.view.Inverted();
    Math::Vector3 position = world.GetTranslation();
    Math::Quaternion rotation = world.GetRotation();

    auto* transform = camera->GetOwner()->GetComponent<TransformComponent>();
    if (transform) {
      transform->SetPosition(position);
      transform->SetRotation(rotation);
    }
  }
}

void EditorLayer::DrawFpsCounter() {
  ImGui::Begin("Performance");
  const ImGuiIO& io = ImGui::GetIO();
  ImGui::Text("%.1f FPS (%.3f ms)", io.Framerate, 1000.0f / io.Framerate);
  ImGui::End();
}

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
      int light_count = CountPointLightsInScene();
      bool can_add = light_count < MAX_POINT_LIGHTS;
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
      } else if (auto* model = dynamic_cast<ModelComponent*>(comp.get())) {
        if (ImGui::CollapsingHeader("ModelComponent")) DrawModelComponentInspector(model);
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

void EditorLayer::DrawRenderPipelinePanel(ID3D12GraphicsCommandList* cmd) {
  ImGui::Begin("Render Pipeline", &show_render_pipeline_);

  auto* graph = graphic_->GetRenderGraph();
  auto& handles = graphic_->GetPreviewHandles();
  float width = ImGui::GetContentRegionAvail().x;
  float aspect = static_cast<float>(graphic_->GetFrameBufferWidth()) / static_cast<float>(graphic_->GetFrameBufferHeight());
  ImVec2 size(width, width / aspect);

  struct Entry {
    const char* label;
    RenderGraphHandle handle;
  };
  Entry entries[] = {
    {"Scene RT", handles.scene_rt},
    {"Normal", handles.normal_preview_rt},
    {"Linear Depth", handles.linear_depth_preview_rt},
    {"Depth", handles.depth_preview_rt},
    {"ToneMapping", handles.tonemap_rt},
  };

  for (auto& [label, handle] : entries) {
    graph->TransitionForRead(cmd, handle);
    ImGui::Text("%s", label);
    auto gpu = graph->GetSrvGpuHandle(handle);
    ImGui::Image(static_cast<ImTextureID>(gpu.ptr), size);

    if (handle == handles.depth_preview_rt) {
      auto& cfg = graphic_->GetDepthPreviewConfig();
      ImGui::DragFloat("Near", &cfg.near_plane, 0.01f, 0.001f, cfg.far_plane);
      ImGui::DragFloat("Far", &cfg.far_plane, 1.0f, cfg.near_plane, 100000.0f);
    }

    ImGui::Separator();
  }

  ImGui::End();
}

void EditorLayer::DrawShadowMapPanel(ID3D12GraphicsCommandList* cmd) {
  ImGui::Begin("Shadow Map", &show_shadow_map_);

  auto* graph = graphic_->GetRenderGraph();
  auto& handles = graphic_->GetPreviewHandles();

  if (handles.shadow_map_count > 0) {
    float total_width = ImGui::GetContentRegionAvail().x;
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float map_width = (total_width - spacing * (handles.shadow_map_count - 1)) / handles.shadow_map_count;
    ImVec2 size(map_width, map_width);

    for (uint32_t i = 0; i < handles.shadow_map_count; ++i) {
      auto handle = handles.shadow_maps[i];
      if (handle == RenderGraphHandle::Invalid) continue;
      graph->TransitionForRead(cmd, handle);

      if (i > 0) ImGui::SameLine();
      ImGui::BeginGroup();
      char label[32];
      std::snprintf(label, sizeof(label), "Cascade %u", i);
      ImGui::Text("%s", label);
      auto gpu = graph->GetSrvGpuHandle(handle);
      ImGui::Image(static_cast<ImTextureID>(gpu.ptr), size);
      ImGui::EndGroup();
    }
  } else {
    ImGui::TextDisabled("Shadow map not available");
  }

  ImGui::End();
}

void EditorLayer::DrawModelComponentInspector(ModelComponent* model) {
  auto* data = model->GetModelData().get();
  ImGui::Text("Model: %s", data ? data->path.c_str() : "None");
  if (data) {
    ImGui::Text("Sub-meshes: %d", static_cast<int>(data->sub_meshes.size()));
    ImGui::Text("Materials: %d", static_cast<int>(data->surface_materials.size()));
    ImGui::Text("Textures: %d", static_cast<int>(data->texture_handles_.size()));
  }

  auto shader_name = ShaderRegistry::GetName(model->GetShaderId());
  ImGui::Text("Shader: %.*s", static_cast<int>(shader_name.size()), shader_name.data());
  ImGui::Text("Model Scale: %.3f", model->GetModelScale());
  if (data) {
    ImGui::Text("Min Y: %.3f", data->min_y);
  }
}

void EditorLayer::RebuildFontAtlas(float scale) {
  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->Clear();

  ImFontConfig config;
  config.SizePixels = 13.0f * scale;
  io.Fonts->AddFontDefault(&config);
  io.Fonts->Build();

  ImGui_ImplDX12_InvalidateDeviceObjects();
  ImGui_ImplDX12_CreateDeviceObjects();
}

namespace {

void ScaleDockNodeRecursive(ImGuiDockNode* node, float ratio) {
  if (!node) return;
  node->SizeRef = ImVec2(node->SizeRef.x * ratio, node->SizeRef.y * ratio);
  ScaleDockNodeRecursive(node->ChildNodes[0], ratio);
  ScaleDockNodeRecursive(node->ChildNodes[1], ratio);
}

}  // namespace

void EditorLayer::ScaleExistingWindows(float ratio) {
  ImGuiContext& g = *ImGui::GetCurrentContext();

  for (ImGuiWindow* window : g.Windows) {
    if (window->DockNode) continue;
    window->Pos = ImVec2(window->Pos.x * ratio, window->Pos.y * ratio);
    window->Size = ImVec2(window->Size.x * ratio, window->Size.y * ratio);
    window->SizeFull = ImVec2(window->SizeFull.x * ratio, window->SizeFull.y * ratio);
  }

  ImGuiDockContext& dc = g.DockContext;
  for (int n = 0; n < dc.Nodes.Data.Size; ++n) {
    auto* node = static_cast<ImGuiDockNode*>(dc.Nodes.Data[n].val_p);
    if (node && node->IsRootNode()) {
      ScaleDockNodeRecursive(node, ratio);
    }
  }
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

void EditorLayer::ApplyLoadedShadowSettings() {
  if (!save_status_success_ || !scene_) return;
  auto& shadow = scene_->GetShadowSetting();
  pending_cascade_count_ = shadow.GetCascadeCount();
  pending_shadow_resolution_ = shadow.GetResolution();
}

void EditorLayer::UpdateScaling() {
  UINT width = graphic_->GetFrameBufferWidth();
  if (width == last_scaled_width_) return;

  float scale = static_cast<float>(width) / 1920.0f;
  scale = std::clamp(scale, 0.5f, 3.0f);

  if (ui_scale_ > 0.0f) {
    ScaleExistingWindows(scale / ui_scale_);
  }

  RebuildFontAtlas(scale);

  ImGui::GetStyle() = base_style_;
  ImGui::GetStyle().ScaleAllSizes(scale);

  last_scaled_width_ = width;
  ui_scale_ = scale;
}

void EditorLayer::CreateMeshGameObject(const char* name, DefaultMesh mesh, ShaderId shader) {
  if (!scene_) return;
  auto* go = scene_->CreateGameObject(name);
  go->AddComponent<MeshRenderer>(MeshRenderer::Props{
    .mesh_type = mesh,
    .shader_id = shader,
  });
  selected_object_ = go;
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
