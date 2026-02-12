#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d12.h>
#include <imgui.h>

#include <string>
#include <vector>

#include "Framework/Event/event_scope.hpp"
#include "Graphic/Descriptor/descriptor_heap_allocator.h"

enum class LoadScope : uint8_t;

class EventBus;
class Graphic;
class IScene;
class GameObject;
class TransformComponent;
class SpriteRenderer;
class UISpriteRenderer;
class TextRenderer;
class UITextRenderer;
class MeshRenderer;
class CameraComponent;
class ModelComponent;
class PointLightComponent;

class DebugDrawer;

class EditorLayer {
 public:
  void Initialize(HWND hwnd, Graphic& graphic);
  void Shutdown(Graphic& graphic);
  void BeginFrame();
  void Render(ID3D12GraphicsCommandList* cmd);
  void SetScene(IScene* scene);
  void SetDebugDrawer(DebugDrawer* drawer);
  void SubscribeEvents(EventBus& bus);
  bool WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

 private:
  void DrawDockSpace();
  void DrawFpsCounter();
  void DrawHierarchy();
  void DrawInspector();
  void DrawTransformInspector(TransformComponent* transform);
  void DrawSpriteRendererInspector(SpriteRenderer* renderer);
  void DrawUISpriteRendererInspector(UISpriteRenderer* renderer);
  void DrawTextRendererInspector(TextRenderer* renderer);
  void DrawUITextRendererInspector(UITextRenderer* renderer);
  void DrawMeshRendererInspector(MeshRenderer* renderer);
  void DrawModelComponentInspector(ModelComponent* model);
  void DrawCameraInspector(CameraComponent* camera);
  void DrawPointLightInspector(PointLightComponent* light);
  void DrawMainMenu();
  void DrawSceneMenu();
  void DrawSaveSceneModal();
  void DrawDumpSettingModal();
  void DrawLoadSceneModal();
  void DrawGameObjectNode(GameObject* go);
  void DrawSceneSettings();
  void DrawDebugPanel();
  void DrawRenderPipelinePanel(ID3D12GraphicsCommandList* cmd);
  void DrawShadowMapPanel(ID3D12GraphicsCommandList* cmd);
  void RebuildFontAtlas(float scale);
  void ScaleExistingWindows(float ratio);
  void UpdateScaling();
  void ScanSceneFiles();
  void ApplyLoadedShadowSettings();

  Graphic* graphic_ = nullptr;
  IScene* scene_ = nullptr;
  DebugDrawer* debug_drawer_ = nullptr;
  GameObject* selected_object_ = nullptr;
  DescriptorHeapAllocator::Allocation imgui_font_srv_;

  bool show_performance_ = true;
  bool show_hierarchy_ = true;
  bool show_inspector_ = true;
  bool show_scene_settings_ = true;
  bool show_debug_ = true;
  bool show_render_pipeline_ = true;
  bool show_shadow_map_ = false;

  float ui_scale_ = 1.0f;
  UINT last_scaled_width_ = 0;
  ImGuiStyle base_style_{};

  EventScope event_scope_;

  bool debug_draw_enabled_ = true;
  float debug_draw_opacity_ = 1.0f;

  uint32_t pending_shadow_resolution_ = 0;
  uint32_t pending_cascade_count_ = 0;

  bool show_save_scene_modal_ = false;
  bool show_dump_setting_modal_ = false;
  bool save_and_dump_ = false;
  char scene_name_buffer_[256] = "Untitled";
  float save_status_timer_ = 0.0f;
  bool save_status_success_ = false;

  // Load scene state
  bool show_load_scene_modal_ = false;
  std::string pending_load_path_;
  LoadScope pending_load_scope_{};
  int selected_scene_index_ = -1;
  std::vector<std::string> scene_file_list_;
};
