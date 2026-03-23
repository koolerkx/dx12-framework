/**
 * @file empty_scene.h
 * @brief A empty scene used for creation in-game
 */
#pragma once

#include "Component/Renderer/mesh_renderer.h"
#include "Component/camera_component.h"
#include "Framework/Core/color.h"
#include "Scripts/free_camera_controller.h"
#include "scene.h"
#include "scene_key.h"

constexpr const char* SKYBOX_PATH = "Content/skybox/sunflowers_puresky_standard_cubemap_4k.hdr";

class EmptyScene : public IScene {
 public:
  void OnEnter(AssetManager& asset_manager) override {
    SetSceneName("Untitled");

    GetBackgroundSetting().SetSkybox(SKYBOX_PATH, asset_manager);

    auto* camera_obj = CreateGameObject("MainCamera", {.position = {0, 5, -10}, .rotation_degrees = {30.0f, 0, 0}});
    auto* camera = camera_obj->AddComponent<CameraComponent>();
    camera_obj->AddComponent<FreeCameraController>(FreeCameraController::Props{
      .movement_speed = 15.0f,
      .rotation_speed = 1.5f,
      .smoothness = 8.0f,
    });
    GetCameraSetting().Register(camera);

    auto* cube = CreateGameObject("Cube");
    cube->AddComponent<MeshRenderer>(MeshRenderer::Props{
      .mesh_type = DefaultMesh::Cube,
      .color = colors::White,
    });
  }

  void OnDebugDraw(DebugDrawer& drawer) override {
    DebugDrawer::GridConfig grid_config;
    grid_config.size = 20.0f;
    grid_config.cell_size = 1.0f;
    grid_config.y_level = 0.0f;
    grid_config.color = colors::Gray;
    drawer.DrawGrid(grid_config);

    DebugDrawer::AxisGizmoConfig axis_config;
    axis_config.position = Vector3::Zero;
    axis_config.length = 2.0f;
    drawer.DrawAxisGizmo(axis_config);
  }

  void OnExit() override {
  }
};

template <>
struct SceneKeyTrait<EmptyScene> {
  static constexpr std::string_view NAME = "empty";
  static constexpr SceneKey KEY = MakeSceneKey(NAME);
};
