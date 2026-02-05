#include "editor_layer.h"

#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

#include "Game/Component/Renderer/sprite_renderer.h"
#include "Game/Component/Renderer/ui_sprite_renderer.h"
#include "Game/Component/transform_component.h"
#include "Game/game_object.h"
#include "Game/scene.h"
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
}

void EditorLayer::Shutdown(Graphic& graphic) {
  ImGui_ImplDX12_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  if (imgui_font_srv_.IsValid()) {
    graphic.GetDescriptorHeapManager().GetSrvStaticAllocator().FreeImmediate(imgui_font_srv_.index, 1);
    imgui_font_srv_ = {};
  }
}

void EditorLayer::BeginFrame() {
  ImGui_ImplDX12_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
}

void EditorLayer::Render(ID3D12GraphicsCommandList* cmd) {
  DrawDockSpace();
  DrawFpsCounter();
  DrawHierarchy();
  DrawInspector();

  ImGui::Render();
  ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmd);
}

void EditorLayer::SetScene(IScene* scene) {
  scene_ = scene;
  selected_object_ = nullptr;
}

bool EditorLayer::WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) return true;

  const ImGuiIO& io = ImGui::GetIO();
  if (io.WantCaptureMouse &&
      (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_MOUSEWHEEL || msg == WM_MOUSEMOVE))
    return true;
  if (io.WantCaptureKeyboard && (msg == WM_KEYDOWN || msg == WM_KEYUP || msg == WM_CHAR)) return true;

  return false;
}

void EditorLayer::DrawDockSpace() {
  ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
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
    for (const auto& go : scene_->GetGameObjects()) {
      if (!go || go->GetParent() != nullptr) continue;
      DrawGameObjectNode(go.get());
    }
  }

  ImGui::End();
}

void EditorLayer::DrawGameObjectNode(GameObject* go) {
  if (!go) return;

  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
  if (go->GetChildren().empty()) flags |= ImGuiTreeNodeFlags_Leaf;
  if (go == selected_object_) flags |= ImGuiTreeNodeFlags_Selected;

  bool open = ImGui::TreeNodeEx(static_cast<void*>(go), flags, "%s", go->GetName().c_str());

  if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) selected_object_ = go;

  if (open) {
    for (auto* child : go->GetChildren()) {
      DrawGameObjectNode(child);
    }
    ImGui::TreePop();
  }
}

void EditorLayer::DrawInspector() {
  ImGui::Begin("Inspector");

  if (selected_object_) {
    ImGui::Text("Name: %s", selected_object_->GetName().c_str());
    ImGui::Separator();

    for (const auto& comp : selected_object_->GetComponents()) {
      if (!comp) continue;

      if (auto* transform = dynamic_cast<TransformComponent*>(comp.get())) {
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
          DrawTransformInspector(transform);
        }
        continue;
      }

      if (ImGui::CollapsingHeader(comp->GetTypeName())) {
        auto* sprite = dynamic_cast<SpriteRenderer*>(comp.get());
        auto* ui_sprite = dynamic_cast<UISpriteRenderer*>(comp.get());

        if (sprite) {
          Math::Vector2 pivot = sprite->GetPivot();
          if (ImGui::DragFloat2("Sprite Pivot", &pivot.x, 0.01f, 0.0f, 1.0f)) sprite->SetPivot(pivot);
        }

        if (ui_sprite) {
          Math::Vector2 pivot = ui_sprite->GetPivot();
          if (ImGui::DragFloat2("UI Pivot", &pivot.x, 0.01f, 0.0f, 1.0f)) ui_sprite->SetPivot(pivot);
        }
      }
    }
  } else {
    ImGui::TextDisabled("No object selected");
  }

  ImGui::End();
}

void EditorLayer::DrawTransformInspector(TransformComponent* transform) {
  Math::Vector3 position = transform->GetPosition();
  if (ImGui::DragFloat3("Position", &position.x, 0.1f)) transform->SetPosition(position);

  Math::Vector3 euler_rad = transform->GetRotation().ToEulerAngles();
  Math::Vector3 euler_deg = {
    Math::ToDegrees(euler_rad.x),
    Math::ToDegrees(euler_rad.y),
    Math::ToDegrees(euler_rad.z),
  };
  if (ImGui::DragFloat3("Rotation", &euler_deg.x, 0.5f)) transform->SetRotationEulerDegree(euler_deg);

  Math::Vector3 scale = transform->GetScale();
  if (ImGui::DragFloat3("Scale", &scale.x, 0.01f)) transform->SetScale(scale);

  Math::Vector3 pivot = transform->GetPivot();
  if (ImGui::DragFloat3("Pivot", &pivot.x, 0.1f)) transform->SetPivot(pivot);
}
