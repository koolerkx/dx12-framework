#include "city_scene.h"

#include <unordered_map>

#include "Asset/asset_manager.h"
#include "Component/Renderer/instanced_model_renderer.h"
#include "Component/Renderer/mesh_renderer.h"
#include "Component/camera_component.h"
#include "Component/transform_component.h"
#include "Debug/debug_drawer.h"
#include "Framework/Core/color.h"
#include "Framework/Event/input_events.h"
#include "Framework/Logging/logger.h"
#include "Framework/Math/Math.h"
#include "Map/map_loader.h"
#include "Scripts/free_camera_controller.h"
#include "scene_id.h"
#include "scene_manager.h"

using Math::Matrix4;
using Math::Vector3;

namespace {

Matrix4 BuildWorldMatrix(const MapItemTransform& t, float y_offset, float origin_x, float origin_z) {
  float rotation_rad = t.rotation_deg * (DirectX::XM_PI / 180.0f);
  // Assimp ConvertToLeftHanded flips Z, making models face -Z; Unity expects +Z forward
  static const Matrix4 FBX_FACING_FIX = Matrix4::CreateRotationY(DirectX::XM_PI);
  return FBX_FACING_FIX * Matrix4::CreateScale(Vector3(t.scale_x, 1.0f, t.scale_z)) * Matrix4::CreateRotationY(rotation_rad) *
         Matrix4::CreateTranslation(Vector3(t.x + origin_x, y_offset, t.z + origin_z));
}

}  // namespace

void CityScene::OnEnter(AssetManager& asset_manager) {
  SetupCamera();

  GetBackgroundSetting().SetSkybox("Content/skybox/sunflowers_puresky_standard_cubemap_4k.hdr", asset_manager);
  auto& light = GetLightSetting();
  light.SetAzimuth(45.0f);
  light.SetElevation(55.0f);

  auto map_data = MapLoader::Load("Content/map/City.yaml");
  if (!map_data) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Game, Logger::Here(), "[CityScene] Failed to load map");
    return;
  }

  constexpr float FBX_UNIT_SCALE = 0.01f;

  std::unordered_map<std::string, std::shared_ptr<ModelData>> model_cache;
  for (const auto& res : map_data->mesh_resources) {
    auto model = asset_manager.LoadModel(res.path, FBX_UNIT_SCALE);
    if (model) {
      model_cache[res.id] = model;
    } else {
      Logger::LogFormat(LogLevel::Warn, LogCategory::Game, Logger::Here(), "[CityScene] Failed to load mesh: {}", res.path);
    }
  }

  auto* map_root = CreateGameObject("Map");

  for (const auto& layer : map_data->layers) {
    auto* layer_go = CreateGameObject(layer.id);
    layer_go->SetParent(map_root);

    // group by mesh to instance
    std::unordered_map<std::string, std::vector<InstancedModelRenderer::InstanceEntry>> grouped;
    for (size_t i = 0; i < layer.items.size(); ++i) {
      const auto& item = layer.items[i];
      std::string instance_id = layer.id + "_" + item.mesh_id + "_" + std::to_string(i);

      InstancedModelRenderer::InstanceEntry entry;
      entry.id = std::move(instance_id);
      entry.props.world = BuildWorldMatrix(item.transform, layer.y_offset, map_data->origin_x, map_data->origin_z);
      entry.props.color = {1.0f, 1.0f, 1.0f, 1.0f};

      grouped[item.mesh_id].push_back(std::move(entry));
    }

    for (auto& [mesh_id, instances] : grouped) {
      auto it = model_cache.find(mesh_id);
      if (it == model_cache.end()) continue;

      std::string go_name = layer.id + "_" + mesh_id + "_instances";
      auto* instance_go = CreateGameObject(go_name);
      instance_go->SetParent(layer_go);

      instance_go->AddComponent<InstancedModelRenderer>(InstancedModelRenderer::Props{
        .model = it->second,
        .instances = std::move(instances),
      });
    }
  }

  Logger::LogFormat(LogLevel::Info,
    LogCategory::Game,
    Logger::Here(),
    "[CityScene] Map loaded: {} layers, {} mesh resources",
    map_data->layers.size(),
    map_data->mesh_resources.size());

  CreateScanlineCube();

  auto& bus = *GetContext()->GetEventBus();
  GetEventScope().Subscribe<KeyDownEvent>(bus, [this](const KeyDownEvent& e) {
    if (e.key == Keyboard::KeyCode::F1) GetContext()->GetSceneManager()->RequestLoad(SceneId::TEST_SCENE);
    if (e.key == Keyboard::KeyCode::F2) GetContext()->GetSceneManager()->RequestLoad(SceneId::MODEL_SCENE);
  });
}

void CityScene::OnExit() {
}

void CityScene::OnDebugDraw(DebugDrawer& drawer) {
  DebugDrawer::GridConfig grid_config;
  grid_config.size = 30.0f;
  grid_config.cell_size = 1.0f;
  grid_config.y_level = 0.0f;
  grid_config.color = colors::Gray;
  drawer.DrawGrid(grid_config);

  DebugDrawer::AxisGizmoConfig axis_config;
  axis_config.position = Vector3::Zero;
  axis_config.length = 2.0f;
  drawer.DrawAxisGizmo(axis_config);
}

void CityScene::CreateScanlineCube() {
  auto* cube = CreateGameObject("ScanlineCube", {.position = {0, 5, 0}});
  auto* renderer = cube->AddComponent<MeshRenderer>(MeshRenderer::Props{
    .mesh_type = DefaultMesh::Cube,
  });
  renderer->SetShaderWithParams<Graphics::ScanlineCubeShader>({
    .primary_r = 0.0f,
    .primary_g = 0.8f,
    .primary_b = 1.0f,
    .grid_divisions = 4.0f,
    .secondary_r = 1.0f,
    .secondary_g = 0.2f,
    .secondary_b = 0.8f,
    .grid_line_width = 0.05f,
    .scanline_speed = 2.0f,
    .scanline_width = 0.15f,
    .glow_intensity = 1.5f,
    .edge_glow_width = 0.02f,
  });
}

void CityScene::SetupCamera() {
  auto* camera_obj = CreateGameObject("MainCamera", {.position = {5, 20, -20}, .rotation_degrees = {45.0f, 0, 0}});
  auto* camera = camera_obj->AddComponent<CameraComponent>();
  camera_obj->AddComponent<FreeCameraController>(FreeCameraController::Props{
    .movement_speed = 20.0f,
    .rotation_speed = 1.5f,
    .smoothness = 8.0f,
  });
  GetCameraSetting().Register(camera);
}
