// imgui.h must precede ImGuizmo.h (ImGuizmo depends on ImGui types)
#include <ImGuizmo.h>
#include <imgui.h>

#include <cstdio>

#include "Framework/Input/input.h"
#include "Game/Component/camera_component.h"
#include "Game/Component/transform_component.h"
#include "Game/scene.h"
#include "Graphic/Presentation/swapchain_manager.h"
#include "Graphic/Render/render_graph.h"
#include "Graphic/graphic.h"
#include "editor_layer.h"


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
