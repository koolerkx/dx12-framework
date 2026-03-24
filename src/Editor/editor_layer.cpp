#include "editor_layer.h"

#include <ImGuizmo.h>
#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>

#include <algorithm>

#include "Framework/Event/event_bus.hpp"
#include "Framework/Event/input_events.h"
#include "Framework/Input/input.h"
#include "Framework/Input/keyboard.h"
#include "Game/Debug/debug_drawer.h"
#include "Game/Serialization/scene_serializer.h"
#include "Game/game_object.h"
#include "Game/scene.h"
#include "Game/scene_events.h"
#include "Graphic/Descriptor/descriptor_heap_manager.h"
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

void EditorLayer::DrawFpsCounter() {
  ImGui::Begin("Performance");
  const ImGuiIO& io = ImGui::GetIO();
  ImGui::Text("%.1f FPS (%.3f ms)", io.Framerate, 1000.0f / io.Framerate);
  ImGui::End();
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
