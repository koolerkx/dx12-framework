#include "city_scene.h"

#include <algorithm>
#include <unordered_map>

#include "Asset/asset_manager.h"
#include "Component/Collider/box_collider_component.h"
#include "Component/Renderer/instanced_model_renderer.h"
#include "Component/Renderer/mesh_renderer.h"
#include "Component/camera_component.h"
#include "Component/spawn_point_component.h"
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
  std::vector<Math::AABB> obstacle_bounds;

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

    bool has_colliders = (layer.id == "object" || layer.id == "spawn");

    for (auto& [mesh_id, instances] : grouped) {
      auto it = model_cache.find(mesh_id);
      if (it == model_cache.end()) continue;

      auto& model = it->second;

      if (has_colliders) {
        for (const auto& entry : instances) {
          auto* collider_go = CreateGameObject(entry.id + "_collider");
          collider_go->SetParent(layer_go);
          auto* collider = collider_go->AddComponent<BoxColliderComponent>(model->bounds, entry.props.world);
          obstacle_bounds.push_back(collider->GetWorldBounds());
        }
      }

      std::string go_name = layer.id + "_" + mesh_id + "_instances";
      auto* instance_go = CreateGameObject(go_name);
      instance_go->SetParent(layer_go);

      instance_go->AddComponent<InstancedModelRenderer>(InstancedModelRenderer::Props{
        .model = model,
        .instances = std::move(instances),
      });
    }
  }

  nav_grid_.Build(*map_data, obstacle_bounds, {.cell_size = 0.25f, .show_debug_grid = true});

  Logger::LogFormat(LogLevel::Info,
    LogCategory::Game,
    Logger::Here(),
    "[CityScene] Map loaded: {} layers, {} mesh resources, nav grid {}x{}",
    map_data->layers.size(),
    map_data->mesh_resources.size(),
    nav_grid_.GetWidth(),
    nav_grid_.GetHeight());

  CreateSpawnCubes(*map_data);

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

  nav_grid_.DebugDraw(drawer, 0.1f);
}

void CityScene::CreateSpawnCubes(const MapData& map_data) {
  auto spawn_it = std::ranges::find_if(map_data.layers, [](const MapLayer& layer) { return layer.id == "spawn"; });
  if (spawn_it == map_data.layers.end()) return;

  constexpr float CUBE_Y_OFFSET = 1.0f;
  constexpr float CUBE_SCALE = 0.5f;

  for (size_t i = 0; i < spawn_it->items.size(); ++i) {
    const auto& item = spawn_it->items[i];
    SpawnType type = (i == 0) ? SpawnType::Player : SpawnType::Enemy;

    float x = item.transform.x + map_data.origin_x;
    float z = item.transform.z + map_data.origin_z;
    float y = spawn_it->y_offset + CUBE_Y_OFFSET;

    std::string name = "SpawnPoint_" + std::to_string(i);
    auto* go = CreateGameObject(name, {.position = {x, y, z}, .scale = {CUBE_SCALE, CUBE_SCALE, CUBE_SCALE}});
    go->AddComponent<SpawnPointComponent>(SpawnPointComponent::Props{.type = type});
  }
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
