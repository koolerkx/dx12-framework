#include "editor_layer.h"

#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>

#include <algorithm>

#include "Framework/Core/utils.h"
#include "Game/Component/Renderer/mesh_renderer.h"
#include "Game/Component/Renderer/sprite_renderer.h"
#include "Game/Component/Renderer/text_renderer.h"
#include "Game/Component/Renderer/ui_sprite_renderer.h"
#include "Game/Component/Renderer/ui_text_renderer.h"
#include "Game/Component/transform_component.h"
#include "Game/game_object.h"
#include "Game/scene.h"
#include "Game/scene_events.h"
#include "Graphic/Descriptor/descriptor_heap_manager.h"
#include "Graphic/Pipeline/shader_registry.h"
#include "Graphic/Render/render_graph.h"
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
  if (pending_shadow_resolution_ > 0) {
    graphic_->WaitForGpuIdle();
    graphic_->SetShadowMapResolution(pending_shadow_resolution_);
    pending_shadow_resolution_ = 0;
  }

  ImGui_ImplDX12_NewFrame();
  ImGui_ImplWin32_NewFrame();
  UpdateScaling();
  ImGui::NewFrame();
}

void EditorLayer::Render(ID3D12GraphicsCommandList* cmd) {
  DrawDockSpace();
  DrawMainMenu();
  if (show_performance_) DrawFpsCounter();
  if (show_hierarchy_) DrawHierarchy();
  if (show_inspector_) DrawInspector();
  if (show_scene_settings_) DrawSceneSettings();
  if (show_debug_) DrawDebugPanel();
  if (show_render_pipeline_) DrawRenderPipelinePanel(cmd);
  if (show_shadow_map_) DrawShadowMapPanel(cmd);

  ImGui::Render();
  ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmd);
}

void EditorLayer::SetScene(IScene* scene) {
  if (scene_ == scene) return;
  scene_ = scene;
  selected_object_ = nullptr;
}

void EditorLayer::SubscribeEvents(EventBus& bus) {
  event_scope_.Subscribe<SceneChangedEvent>(bus, [this](const SceneChangedEvent& e) { SetScene(e.new_scene); });
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

void EditorLayer::DrawMainMenu() {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("Window")) {
      ImGui::MenuItem("Performance", nullptr, &show_performance_);
      ImGui::MenuItem("Hierarchy", nullptr, &show_hierarchy_);
      ImGui::MenuItem("Inspector", nullptr, &show_inspector_);
      ImGui::MenuItem("Scene Settings", nullptr, &show_scene_settings_);
      ImGui::MenuItem("Debug", nullptr, &show_debug_);
      ImGui::MenuItem("Render Pipeline", nullptr, &show_render_pipeline_);
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
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

namespace {

void DrawColorEditor(const char* label, Math::Vector4& color) {
  ImGui::ColorEdit4(label, &color.x);
}

void DrawRenderSettingsEditor(Rendering::RenderSettings& settings, bool show_depth) {
  static const char* kBlendModeNames[] = {"Opaque", "AlphaBlend", "Additive", "Premultiplied"};
  int blend = static_cast<int>(settings.blend_mode);
  if (ImGui::Combo("Blend Mode", &blend, kBlendModeNames, IM_ARRAYSIZE(kBlendModeNames))) {
    settings.blend_mode = static_cast<Rendering::BlendMode>(blend);
  }

  static const char* kSamplerNames[] = {"PointWrap", "LinearWrap", "AnisotropicWrap", "PointClamp", "LinearClamp"};
  int sampler = static_cast<int>(settings.sampler_type);
  if (ImGui::Combo("Sampler", &sampler, kSamplerNames, IM_ARRAYSIZE(kSamplerNames))) {
    settings.sampler_type = static_cast<Rendering::SamplerType>(sampler);
  }

  if (show_depth) {
    if (ImGui::Checkbox("Depth Test", &settings.depth_test)) {
      if (!settings.depth_test) settings.depth_write = false;
    }
    ImGui::BeginDisabled(!settings.depth_test);
    ImGui::Checkbox("Depth Write", &settings.depth_write);
    ImGui::EndDisabled();
    ImGui::Checkbox("Double Sided", &settings.double_sided);
  }
}

void DrawBillboardEditor(Billboard::Mode& mode) {
  static const char* kBillboardNames[] = {"None", "Cylindrical", "Spherical"};
  int current = static_cast<int>(mode);
  if (ImGui::Combo("Billboard", &current, kBillboardNames, IM_ARRAYSIZE(kBillboardNames))) {
    mode = static_cast<Billboard::Mode>(current);
  }
}

void DrawRenderLayerEditor(RenderLayer& layer) {
  static const char* kLayerNames[] = {"Opaque", "Transparent"};
  int current = static_cast<int>(layer);
  if (current >= IM_ARRAYSIZE(kLayerNames)) current = 1;
  if (ImGui::Combo("Render Layer", &current, kLayerNames, IM_ARRAYSIZE(kLayerNames))) {
    layer = static_cast<RenderLayer>(current);
  }
}

void DrawTextProperties(std::wstring& text,
  Font::FontFamily& font_family,
  float& pixel_size,
  Text::HorizontalAlign& h_align,
  float& line_spacing,
  float& letter_spacing) {
  std::string utf8_text = utils::wstring_to_utf8(text);
  char buf[512];
  strncpy_s(buf, utf8_text.c_str(), sizeof(buf) - 1);
  if (ImGui::InputText("Text", buf, sizeof(buf))) {
    text = utils::utf8_to_wstring(std::string(buf));
  }

  static const char* kFontNames[] = {"ZenOldMincho"};
  int font = static_cast<int>(font_family);
  if (ImGui::Combo("Font", &font, kFontNames, IM_ARRAYSIZE(kFontNames))) {
    font_family = static_cast<Font::FontFamily>(font);
  }

  ImGui::DragFloat("Pixel Size", &pixel_size, 0.5f, 1.0f, 256.0f);

  static const char* kHAlignNames[] = {"Left", "Center", "Right"};
  int h = static_cast<int>(h_align);
  if (ImGui::Combo("H Align", &h, kHAlignNames, IM_ARRAYSIZE(kHAlignNames))) {
    h_align = static_cast<Text::HorizontalAlign>(h);
  }

  ImGui::DragFloat("Line Spacing", &line_spacing, 0.1f);
  ImGui::DragFloat("Letter Spacing", &letter_spacing, 0.1f);
}

}  // namespace

void EditorLayer::DrawInspector() {
  ImGui::Begin("Inspector");

  if (selected_object_) {
    ImGui::Text("Name: %s", selected_object_->GetName().c_str());
    ImGui::Separator();

    for (const auto& comp : selected_object_->GetComponents()) {
      if (!comp) continue;
      ImGui::PushID(comp.get());

      if (auto* transform = dynamic_cast<TransformComponent*>(comp.get())) {
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
          DrawTransformInspector(transform);
        }
      } else if (auto* sprite = dynamic_cast<SpriteRenderer*>(comp.get())) {
        if (ImGui::CollapsingHeader("SpriteRenderer")) DrawSpriteRendererInspector(sprite);
      } else if (auto* ui_sprite = dynamic_cast<UISpriteRenderer*>(comp.get())) {
        if (ImGui::CollapsingHeader("UISpriteRenderer")) DrawUISpriteRendererInspector(ui_sprite);
      } else if (auto* text_r = dynamic_cast<TextRenderer*>(comp.get())) {
        if (ImGui::CollapsingHeader("TextRenderer")) DrawTextRendererInspector(text_r);
      } else if (auto* ui_text = dynamic_cast<UITextRenderer*>(comp.get())) {
        if (ImGui::CollapsingHeader("UITextRenderer")) DrawUITextRendererInspector(ui_text);
      } else if (auto* mesh = dynamic_cast<MeshRenderer*>(comp.get())) {
        if (ImGui::CollapsingHeader("MeshRenderer")) DrawMeshRendererInspector(mesh);
      } else {
        ImGui::CollapsingHeader(comp->GetTypeName());
      }

      ImGui::PopID();
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

void EditorLayer::DrawSpriteRendererInspector(SpriteRenderer* renderer) {
  auto data = renderer->GetEditorData();

  DrawColorEditor("Color", data.color);
  ImGui::DragFloat2("Size", &data.size.x, 1.0f);
  ImGui::DragFloat2("Pivot", &data.pivot.x, 0.01f, 0.0f, 1.0f);
  ImGui::DragFloat2("UV Offset", &data.uv_offset.x, 0.01f);
  ImGui::DragFloat2("UV Scale", &data.uv_scale.x, 0.01f);
  DrawBillboardEditor(data.billboard_mode);
  DrawRenderLayerEditor(data.render_layer);
  DrawRenderSettingsEditor(data.render_settings, true);

  renderer->ApplyEditorData(data);
}

void EditorLayer::DrawUISpriteRendererInspector(UISpriteRenderer* renderer) {
  auto data = renderer->GetEditorData();

  DrawColorEditor("Color", data.color);
  ImGui::DragFloat2("Size", &data.size.x, 1.0f);
  ImGui::DragFloat2("Pivot", &data.pivot.x, 0.01f, 0.0f, 1.0f);
  ImGui::DragFloat2("UV Offset", &data.uv_offset.x, 0.01f);
  ImGui::DragFloat2("UV Scale", &data.uv_scale.x, 0.01f);
  ImGui::DragInt("Layer ID", &data.layer_id);
  DrawRenderSettingsEditor(data.render_settings, false);

  renderer->ApplyEditorData(data);
}

void EditorLayer::DrawTextRendererInspector(TextRenderer* renderer) {
  auto data = renderer->GetEditorData();

  DrawTextProperties(data.text, data.font_family, data.pixel_size, data.h_align, data.line_spacing, data.letter_spacing);
  DrawColorEditor("Color", data.color);
  DrawBillboardEditor(data.billboard_mode);
  ImGui::DragFloat2("Pivot", &data.pivot.x, 0.01f, 0.0f, 1.0f);
  DrawRenderLayerEditor(data.render_layer);
  DrawRenderSettingsEditor(data.render_settings, false);

  Math::Vector2 size = renderer->GetSize();
  ImGui::Text("Size: %.1f x %.1f", size.x, size.y);

  renderer->ApplyEditorData(data);
}

void EditorLayer::DrawUITextRendererInspector(UITextRenderer* renderer) {
  auto data = renderer->GetEditorData();

  DrawTextProperties(data.text, data.font_family, data.pixel_size, data.h_align, data.line_spacing, data.letter_spacing);
  DrawColorEditor("Color", data.color);
  ImGui::DragInt("Layer ID", &data.layer_id);
  ImGui::DragFloat2("Pivot", &data.pivot.x, 0.01f, 0.0f, 1.0f);
  DrawRenderSettingsEditor(data.render_settings, false);

  Math::Vector2 size = renderer->GetSize();
  ImGui::Text("Size: %.1f x %.1f", size.x, size.y);

  renderer->ApplyEditorData(data);
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

    static constexpr const char* ALGORITHM_LABELS[] = {"Hard", "PCF 3x3", "Poisson Disk", "Rotated Poisson Disk"};
    int algo = static_cast<int>(shadow.GetAlgorithm());
    if (ImGui::Combo("Algorithm", &algo, ALGORITHM_LABELS, IM_ARRAYSIZE(ALGORITHM_LABELS))) {
      shadow.SetAlgorithm(static_cast<ShadowAlgorithm>(algo));
    }

    float depth_bias = shadow.GetDepthBias();
    if (ImGui::DragFloat("Depth Bias", &depth_bias, 0.0001f, 0.0f, 0.1f, "%.4f")) {
      shadow.SetDepthBias(depth_bias);
    }

    float normal_bias = shadow.GetNormalBias();
    if (ImGui::DragFloat("Normal Bias", &normal_bias, 0.001f, 0.0f, 0.5f, "%.3f")) {
      shadow.SetNormalBias(normal_bias);
    }

    float distance = shadow.GetShadowDistance();
    if (ImGui::DragFloat("Shadow Distance", &distance, 1.0f, 1.0f, 1000.0f, "%.0f")) {
      shadow.SetShadowDistance(distance);
    }

    float light_dist = shadow.GetLightDistance();
    if (ImGui::DragFloat("Light Distance", &light_dist, 1.0f, 1.0f, 1000.0f, "%.0f")) {
      shadow.SetLightDistance(light_dist);
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

void EditorLayer::DrawDebugPanel() {
  ImGui::Begin("Debug");

  auto& depth_cfg = graphic_->GetDepthViewConfig();
  auto& hdr_dbg = graphic_->GetHdrDebug();

  static const char* kDisplayViewNames[] = {"Normal", "Scene RT", "Depth Buffer"};
  int current = 0;
  if (hdr_dbg.debug_view) current = 1;
  if (depth_cfg.enabled) current = 2;

  if (ImGui::Combo("Display View", &current, kDisplayViewNames, IM_ARRAYSIZE(kDisplayViewNames))) {
    hdr_dbg.debug_view = (current == 1);
    depth_cfg.enabled = (current == 2);
  }

  if (depth_cfg.enabled) {
    ImGui::DragFloat("Near Plane", &depth_cfg.near_plane, 0.01f, 0.001f, depth_cfg.far_plane);
    ImGui::DragFloat("Far Plane", &depth_cfg.far_plane, 1.0f, depth_cfg.near_plane, 100000.0f);
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
  auto handle = graphic_->GetPreviewHandles().shadow_map;

  if (handle != RenderGraphHandle::Invalid) {
    graph->TransitionForRead(cmd, handle);
    float width = ImGui::GetContentRegionAvail().x;
    ImVec2 size(width, width);
    auto gpu = graph->GetSrvGpuHandle(handle);
    ImGui::Image(static_cast<ImTextureID>(gpu.ptr), size);
  } else {
    ImGui::TextDisabled("Shadow map not available");
  }

  ImGui::End();
}

void EditorLayer::DrawMeshRendererInspector(MeshRenderer* renderer) {
  const Mesh* mesh = renderer->GetMesh();
  ImGui::Text("Mesh: %s", mesh ? "Loaded" : "None");

  Texture* texture = renderer->GetTexture();
  ImGui::Text("Texture: %s", texture ? "Loaded" : "None");

  auto shader_name = ShaderRegistry::GetName(renderer->GetShaderId());
  ImGui::Text("Shader: %.*s", static_cast<int>(shader_name.size()), shader_name.data());

  auto data = renderer->GetEditorData();

  DrawColorEditor("Color", data.color);
  DrawRenderLayerEditor(data.render_layer);
  DrawRenderSettingsEditor(data.render_settings, true);

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

    std::string preview;
    for (const auto& [label, tag] : TAG_ENTRIES) {
      if (HasTag(data.render_tags, tag)) {
        if (!preview.empty()) preview += ", ";
        preview += label;
      }
    }
    if (preview.empty()) preview = "None";

    if (ImGui::BeginCombo("Render Tags", preview.c_str())) {
      for (const auto& [label, tag] : TAG_ENTRIES) {
        bool has = HasTag(data.render_tags, tag);
        if (ImGui::Checkbox(label, &has)) {
          if (has)
            data.render_tags |= static_cast<uint32_t>(tag);
          else
            data.render_tags &= ~static_cast<uint32_t>(tag);
        }
      }
      ImGui::EndCombo();
    }
  }

  renderer->ApplyEditorData(data);
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
