#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d12.h>
#include <imgui.h>

#include <string>
#include <unordered_set>
#include <vector>

#include "Framework/Event/event_scope.hpp"
#include "Graphic/Descriptor/descriptor_heap_allocator.h"
#include "Graphic/Pipeline/shader_types.h"

enum class DefaultMesh;
enum class LoadScope : uint8_t;

class EventBus;
class Game;
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
  void SetGame(Game* game) {
    game_ = game;
  }
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
  void DrawSaveExclusionTree(GameObject* go, bool parent_excluded);
  void DrawDumpSettingModal();
  void DrawLoadSceneModal();
  void DrawGameObjectNode(GameObject* go);
  void DrawAddModelModal();
  void DrawSceneSettings();
  void DrawPlayControls();
  void DrawDebugPanel();
  void DrawEditorSettings();
  void DrawPostFxPanel();
  void DrawRenderPipelinePanel(ID3D12GraphicsCommandList* cmd);
  void DrawShadowMapPanel(ID3D12GraphicsCommandList* cmd);
  void RebuildFontAtlas(float scale);
  void ScaleExistingWindows(float ratio);
  void UpdateScaling();
  void ScanSceneFiles();
  void ScanModelFiles();
  void ScanTextureFiles();
  int CountPointLightsInScene() const;
  void CreateMeshGameObject(const char* name, DefaultMesh mesh, Graphics::ShaderId shader);
  void ApplyPendingModelCreation();
  void ResolveDirtyRenderers();
  void ApplyLoadedShadowSettings();

  Game* game_ = nullptr;
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
  bool show_postfx_ = true;
  bool show_render_pipeline_ = true;
  bool show_shadow_map_ = false;
  bool show_editor_settings_ = false;
  bool imgui_visible_ = true;

  float ui_scale_ = 1.0f;
  UINT last_scaled_width_ = 0;
  ImGuiStyle base_style_{};

  EventScope event_scope_;

  bool debug_draw_enabled_ = true;
  float debug_draw_opacity_ = 1.0f;
  bool wireframe_mode_ = false;

  float transform_position_snap_ = 0.1f;
  float transform_rotation_snap_ = 0.5f;
  float transform_scale_snap_ = 0.01f;

  uint32_t pending_shadow_resolution_ = 0;
  uint32_t pending_cascade_count_ = 0;

  bool show_save_scene_modal_ = false;
  bool show_dump_setting_modal_ = false;
  bool save_and_dump_ = false;
  char scene_name_buffer_[256] = "Untitled";
  std::unordered_set<GameObject*> save_excluded_objects_;
  float save_status_timer_ = 0.0f;
  bool save_status_success_ = false;

  GameObject* pending_delete_ = nullptr;

  bool show_add_model_modal_ = false;
  std::vector<std::string> model_file_list_;
  int selected_model_index_ = -1;

  std::vector<std::string> texture_file_list_;
  std::vector<MeshRenderer*> dirty_renderers_;

  std::string pending_model_path_;

  // Load scene state
  bool show_load_scene_modal_ = false;
  std::string pending_load_path_;
  LoadScope pending_load_scope_{};
  bool pending_load_additive_ = false;
  int selected_scene_index_ = -1;
  std::vector<std::string> scene_file_list_;
};
