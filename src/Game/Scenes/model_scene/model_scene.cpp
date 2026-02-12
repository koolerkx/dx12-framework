#include "model_scene.h"

#include <array>
#include <string>

#include "Asset/asset_manager.h"
#include "Component/Renderer/mesh_renderer.h"
#include "Component/camera_component.h"
#include "Component/model_component.h"
#include "Component/transform_component.h"
#include "Debug/debug_drawer.h"
#include "Framework/Core/color.h"
#include "Framework/Input/input.h"
#include "Framework/Logging/logger.h"
#include "Scripts/free_camera_controller.h"
#include "scene_id.h"
#include "scene_manager.h"

using Math::Vector3;

void ModelScene::OnEnter(AssetManager& asset_manager) {
  SetupCamera();

  GetBackgroundSetting().SetSkybox("Content/skybox/sunflowers_puresky_standard_cubemap_4k.hdr", asset_manager);
  auto& light = GetLightSetting();
  light.SetAzimuth(45.0f);
  light.SetElevation(55.0f);

  auto* ground = CreateGameObject("Ground", {.position = {0, -1, 0}, .scale = {30, 1, 30}});
  ground->AddComponent<MeshRenderer>(MeshRenderer::Props{
    .mesh = asset_manager.GetDefaultMesh(DefaultMesh::Plane),
    .color = {0.3f, 0.3f, 0.3f, 1.0f},
  });

  struct ModelEntry {
    std::string path;
    float x_offset;
    bool split_mesh_to_children = false;
  };

  std::array<ModelEntry, 4> models = {{
    {"Content/models/test.fbx", -9.0f, true},
    {"Content/models/slime.fbx", -3.0f},
    {"Content/models/shinamon.fbx", 3.0f},
    {"Content/models/modified_Drone.fbx", 9.0f},
  }};

  for (const auto& entry : models) {
    auto model_data = asset_manager.LoadModel(entry.path);
    if (!model_data) {
      Logger::LogFormat(LogLevel::Error, LogCategory::Game, Logger::Here(), "[ModelScene] Failed to load: {}", entry.path);
      continue;
    }

    auto* obj = CreateGameObject(entry.path, {.position = {entry.x_offset, 0, 0}});
    obj->AddComponent<ModelComponent>(ModelComponent::Props{.model = model_data, .split_mesh_to_children = entry.split_mesh_to_children});

    Logger::LogFormat(LogLevel::Info,
      LogCategory::Game,
      Logger::Here(),
      "[ModelScene] Loaded: {} ({} sub-meshes)",
      entry.path,
      model_data->sub_meshes.size());
  }
}

void ModelScene::OnPostUpdate(float /*dt*/) {
  auto* input = GetContext()->GetInput();
  if (input && input->GetKeyDown(Keyboard::KeyCode::F2)) {
    GetContext()->GetSceneManager()->RequestLoad(SceneId::TEST_SCENE);
  }
}

void ModelScene::OnDebugDraw(DebugDrawer& drawer) {
  DebugDrawer::GridConfig grid_config;
  grid_config.size = 30.0f;
  grid_config.cell_size = 1.0f;
  grid_config.y_level = -1.0f;
  grid_config.color = colors::Gray;
  drawer.DrawGrid(grid_config);

  DebugDrawer::AxisGizmoConfig axis_config;
  axis_config.position = Vector3::Zero;
  axis_config.length = 2.0f;
  drawer.DrawAxisGizmo(axis_config);
}

void ModelScene::OnExit() {
}

void ModelScene::SetupCamera() {
  auto* camera_obj = CreateGameObject("MainCamera", {.position = {0, 3, -15}});
  auto* camera = camera_obj->AddComponent<CameraComponent>();
  camera_obj->AddComponent<FreeCameraController>(FreeCameraController::Props{
    .movement_speed = 15.0f,
    .rotation_speed = 1.5f,
    .smoothness = 8.0f,
  });
  GetCameraSetting().Register(camera);
}
