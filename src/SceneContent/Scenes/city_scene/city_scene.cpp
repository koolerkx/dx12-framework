#include "city_scene.h"

#include <algorithm>
#include <unordered_map>

#include "Component/Collider/box_collider_component.h"
#include "Component/Renderer/instanced_mesh_renderer.h"
#include "Component/Renderer/mesh_renderer.h"
#include "Component/Renderer/ui_glass_renderer.h"
#include "Component/camera_component.h"
#include "Component/enemy_spawn_component.h"
#include "Component/model_component.h"
#include "Component/player_spawn_component.h"
#include "Component/transform_component.h"
#include "Debug/debug_drawer.h"
#include "Framework/Asset/asset_manager.h"
#include "Framework/Core/color.h"
#include "Framework/Event/input_events.h"
#include "Framework/Logging/logger.h"
#include "Framework/Math/Math.h"
#include "Framework/Render/render_service.h"
#include "Map/map_loader.h"
#include "Scenes/city_scene/base_health_component.h"
#include "Scenes/city_scene/city_scene_config.h"
#include "Scenes/city_scene/city_scene_events.h"
#include "Scenes/city_scene/currency_component.h"
#include "Scenes/city_scene/enemy_spawn_manager_component.h"
#include "Scenes/city_scene/explosion_effect.h"
#include "Scenes/city_scene/floating_text_effect.h"
#include "Scenes/city_scene/game_match_component.h"
#include "Scenes/city_scene/hud_manager_component.h"
#include "Scenes/city_scene/player_control_component.h"
#include "Scenes/city_scene/turret_system.h"
#include "Scenes/city_scene/wave_controller_component.h"
#include "Scenes/city_scene/wave_system.h"
#include "Scripts/camera_shake_controller.h"
#include "Scripts/free_camera_controller.h"
#include "Scripts/screen_effect_controller.h"
#include "game_context.h"
#include "play_state.h"
#include "scene_key.h"
#include "scene_manager.h"

CityScene::CityScene() = default;
CityScene::~CityScene() = default;

namespace cfg = CitySceneConfig;

using Math::Matrix4;
using Math::Vector3;

using Math::Quaternion;

void CityScene::OnEnter(AssetManager& asset_manager) {
  SetupCamera();

  constexpr cfg::LightConfig LIGHT;
  GetBackgroundSetting().SetSkybox(cfg::PATHS.skybox, asset_manager);
  auto& light = GetLightSetting();
  light.SetAzimuth(LIGHT.azimuth);
  light.SetElevation(LIGHT.elevation);

  auto map_data = MapLoader::Load(cfg::PATHS.map);
  if (!map_data) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Game, Logger::Here(), "[CityScene] Failed to load map");
    return;
  }

  std::unordered_map<std::string, std::shared_ptr<ModelData>> model_cache;
  for (const auto& res : map_data->mesh_resources) {
    auto model = asset_manager.LoadModel(res.path, cfg::FBX_UNIT_SCALE);
    if (model) {
      model_cache[res.id] = model;
    } else {
      Logger::LogFormat(LogLevel::Warn, LogCategory::Game, Logger::Here(), "[CityScene] Failed to load mesh: {}", res.path);
    }
  }

  auto* map_root = CreateGameObject("Map");
  std::vector<Math::AABB> obstacle_bounds;

  for (const auto& layer : map_data->layers) {
    if (layer.id == "spawn") continue;

    auto* layer_go = CreateGameObject(layer.id);
    layer_go->SetParent(map_root);
    bool has_colliders = (layer.id == "object");

    for (size_t i = 0; i < layer.items.size(); ++i) {
      const auto& item = layer.items[i];
      auto it = model_cache.find(item.mesh_id);
      if (it == model_cache.end()) continue;

      std::string go_name = layer.id + "_" + item.mesh_id + "_" + std::to_string(i);
      auto* go = CreateGameObject(go_name);
      go->SetParent(layer_go);

      // Assimp ConvertToLeftHanded flips Z → add 180° to compensate
      float rotation_rad = (item.transform.rotation_deg + 180.0f) * (DirectX::XM_PI / 180.0f);
      auto* transform = go->GetTransform();
      transform->SetPosition({
        item.transform.x + map_data->origin_x,
        layer.y_offset,
        item.transform.z + map_data->origin_z,
      });
      transform->SetRotation(Quaternion::CreateFromAxisAngle(Vector3::UnitY, rotation_rad));
      transform->SetScale({item.transform.scale_x, 1.0f, item.transform.scale_z});

      go->AddComponent<ModelComponent>(ModelComponent::Props{
        .model = it->second,
        .shader_id = Shaders::PBR::ID,
      });

      if (has_colliders) {
        auto world = transform->GetWorldMatrix();
        auto* collider = go->AddComponent<BoxColliderComponent>(it->second->bounds, world);
        obstacle_bounds.push_back(collider->GetWorldBounds());
      }
    }
  }

  constexpr cfg::NavGridConfig NAV;
  nav_grid_.Build(*map_data,
    obstacle_bounds,
    {.cell_size = NAV.cell_size, .block_threshold = NAV.block_threshold, .show_debug_grid = NAV.show_debug_grid});

  SpawnBorderWalls(*map_data);
  SetupCameraBounds(*map_data);
  SpawnEnemyManager();
  CreateSpawnCubes(*map_data);

  auto* player = CreateGameObject("Player");
  player->AddComponent<CurrencyComponent>(CurrencyComponent::Props{});
  player->AddComponent<BaseHealthComponent>(BaseHealthComponent::Props{});
  player->AddComponent<GameMatchComponent>(GameMatchComponent::Props{});
  player->AddComponent<WaveControllerComponent>(WaveControllerComponent::Props{});
  player->AddComponent<PlayerControlComponent>(PlayerControlComponent::Props{.nav = &nav_grid_});

  Logger::LogFormat(LogLevel::Info,
    LogCategory::Game,
    Logger::Here(),
    "[CityScene] Map loaded: {} layers, {} mesh resources, nav grid {}x{}",
    map_data->layers.size(),
    map_data->mesh_resources.size(),
    nav_grid_.GetWidth(),
    nav_grid_.GetHeight());

  auto* hud = CreateGameObject("HUD");
  hud->AddComponent<HudManagerComponent>();

  wave_system_ = std::make_unique<WaveSystem>(this);
  turret_system_ = std::make_unique<TurretSystem>(this);

  transition_overlay_.Create(this, "SceneTransitionOverlay");
  if (GetContext()->GetPlayState() == PlayState::Playing) {
    auto* rs = GetContext()->GetRenderService();
    float screen_w = static_cast<float>(rs->GetSceneWidth());
    float screen_h = static_cast<float>(rs->GetSceneHeight());
    transition_overlay_.SetOpaque();
    transition_overlay_.UpdateLayout(screen_w, screen_h);
    transition_overlay_.FadeOut();
  }

  RegisterEventHandlers();

  auto& bus = *GetContext()->GetEventBus();
  GetEventScope().Subscribe<KeyDownEvent>(bus, [this](const KeyDownEvent& e) {
    if (e.key == Keyboard::KeyCode::F1) GetContext()->GetSceneManager()->RequestLoad(MakeSceneKey("test"));
    if (e.key == Keyboard::KeyCode::F2) GetContext()->GetSceneManager()->RequestLoad(MakeSceneKey("model"));
  });

  GetEventScope().Subscribe<RestartGameEvent>(
    bus, [this](const RestartGameEvent&) { GetContext()->GetSceneManager()->RequestLoad(MakeSceneKey("city")); });
}

void CityScene::OnExit() {
  turret_system_.reset();
  wave_system_.reset();
}

void CityScene::OnPreUpdate(float dt) {
  transition_overlay_.Update(dt);
  if (wave_system_) wave_system_->Update(dt);
  if (turret_system_) turret_system_->Update(dt);
}

void CityScene::OnRender(FramePacket& /*packet*/) {
  auto* rs = GetContext()->GetRenderService();
  float screen_w = static_cast<float>(rs->GetSceneWidth());
  float screen_h = static_cast<float>(rs->GetSceneHeight());
  transition_overlay_.UpdateLayout(screen_w, screen_h);
}

void CityScene::OnDebugDraw(DebugDrawer& drawer) {
  constexpr cfg::DebugDrawConfig DBG;

  DebugDrawer::GridConfig grid_config;
  grid_config.size = DBG.grid_size;
  grid_config.cell_size = DBG.grid_cell_size;
  grid_config.y_level = DBG.grid_y_level;
  grid_config.color = colors::Gray;
  drawer.DrawGrid(grid_config);

  DebugDrawer::AxisGizmoConfig axis_config;
  axis_config.position = Vector3::Zero;
  axis_config.length = DBG.axis_length;
  drawer.DrawAxisGizmo(axis_config);

  nav_grid_.DebugDraw(drawer, DBG.nav_grid_y_level);
}

void CityScene::SpawnEnemyManager() {
  auto* enemy_manager = CreateGameObject("EnemyManager");
  enemy_manager->AddComponent<EnemySpawnManagerComponent>(EnemySpawnManagerComponent::Props{
    .nav = &nav_grid_,
  });
}

void CityScene::SpawnBorderWalls(const MapData& map_data) {
  auto bounds = ComputeGroundBounds(map_data);
  if (bounds.min_x > bounds.max_x) return;

  constexpr cfg::BorderWallConfig WALL;

  float wall_min_x = bounds.min_x - WALL.margin;
  float wall_max_x = bounds.max_x;
  float wall_min_z = bounds.min_z - WALL.margin;
  float wall_max_z = bounds.max_z;

  std::vector<MeshInstanceEntry> entries;

  auto add_instance = [&](float x, float z) {
    entries.push_back({
      .world = Matrix4::CreateTranslation({x, WALL.cube_y, z}),
      .color = {1.0f, 1.0f, 1.0f, 1.0f},
    });
  };

  for (float x = wall_min_x; x <= wall_max_x; x += 1.0f) {
    float cx = x + 0.5f;
    add_instance(cx, wall_min_z + 0.5f);
    add_instance(cx, wall_max_z + 0.5f);
  }

  for (float z = wall_min_z + 1.0f; z < wall_max_z; z += 1.0f) {
    float cz = z + 0.5f;
    add_instance(wall_min_x + 0.5f, cz);
    add_instance(wall_max_x + 0.5f, cz);
  }

  auto* wall_obj = CreateGameObject("BorderWalls");
  wall_obj->AddComponent<InstancedMeshRenderer>(InstancedMeshRenderer::Props{
    .mesh_type = DefaultMesh::Cube,
    .instances = std::move(entries),
  });
}

void CityScene::CreateSpawnCubes(const MapData& map_data) {
  auto spawn_it = std::ranges::find_if(map_data.layers, [](const MapLayer& layer) { return layer.id == "spawn"; });
  if (spawn_it == map_data.layers.end()) return;

  constexpr cfg::SpawnCubeConfig SPAWN;

  for (size_t i = 0; i < spawn_it->items.size(); ++i) {
    const auto& item = spawn_it->items[i];
    bool is_player = (i == 0);

    float x = item.transform.x + map_data.origin_x;
    float z = item.transform.z + map_data.origin_z;
    float y = spawn_it->y_offset + SPAWN.y_offset;

    std::string name = "SpawnPoint_" + std::to_string(i);
    auto* go = CreateGameObject(name, {.position = {x, y, z}, .scale = {SPAWN.scale, SPAWN.scale, SPAWN.scale}});

    if (is_player) {
      go->AddComponent<PlayerSpawnComponent>();
    } else {
      go->AddComponent<EnemySpawnComponent>();
    }
  }
}

void CityScene::SetupCamera() {
  const cfg::CameraConfig CAM;
  auto* camera_obj = CreateGameObject("MainCamera", {.position = CAM.position, .rotation_degrees = CAM.rotation_degrees});
  auto* camera = camera_obj->AddComponent<CameraComponent>();
  camera_obj->AddComponent<FreeCameraController>(FreeCameraController::Props{
    .movement_speed = CAM.movement_speed,
    .rotation_speed = CAM.rotation_speed,
    .smoothness = CAM.smoothness,
  });
  camera_obj->AddComponent<CameraShakeController>();
  camera_obj->AddComponent<ScreenEffectController>();
  GetCameraSetting().Register(camera);
}

void CityScene::RegisterEventHandlers() {
  auto& bus = *GetContext()->GetEventBus();
  auto& scope = GetEventScope();

  scope.Subscribe<EntityDeathEvent>(bus, [this](const EntityDeathEvent& e) {
    auto* player = FindGameObject("Player");
    if (!player) return;

    if (auto* match = player->GetComponent<GameMatchComponent>()) match->IncrementKillCount();

    if (auto* currency = player->GetComponent<CurrencyComponent>()) {
      currency->AddGold(e.kill_reward);
      const CitySceneConfig::FloatingTextConfig txt_cfg;
      CitySceneEffect::SpawnRewardText(this, e.position + Math::Vector3(0, txt_cfg.y_offset, 0), e.kill_reward);
    }
  });

  scope.Subscribe<BaseDestroyedEvent>(bus, [this](const BaseDestroyedEvent&) {
    auto* player = FindGameObject("Player");
    if (!player) return;

    if (auto* match = player->GetComponent<GameMatchComponent>()) match->SetGameOver();
  });

  scope.Subscribe<EnemyArrivedEvent>(bus, [this](const EnemyArrivedEvent& e) {
    auto* player = FindGameObject("Player");
    if (!player) return;

    if (auto* health = player->GetComponent<BaseHealthComponent>()) health->TakeDamage();

    const CitySceneConfig::ArrivalExplosionConfig explosion_cfg;
    auto pos = e.position;
    pos.y += explosion_cfg.y_offset;
    CitySceneEffect::SpawnExplosion(this, pos, CitySceneEffect::FromArrivalConfig(explosion_cfg), "ArrivalExplosion");

    const CitySceneConfig::ExplosionSparksConfig sparks_cfg;
    CitySceneEffect::SpawnExplosionSparks(this, pos, CitySceneEffect::FromExplosionSparksConfig(sparks_cfg), "ArrivalSparks");

    const CitySceneConfig::ArrivalScreenEffectConfig fx_cfg;
    auto* camera_go = FindGameObject("MainCamera");
    if (camera_go) {
      if (auto* shake = camera_go->GetComponent<CameraShakeController>()) shake->Trigger(fx_cfg.shake_intensity, fx_cfg.shake_duration);
      if (auto* screen_fx = camera_go->GetComponent<ScreenEffectController>())
        screen_fx->TriggerChromaticAberration(fx_cfg.chromatic_aberration_intensity);
    }
  });
}

void CityScene::SetupCameraBounds(const MapData& map_data) {
  auto xz = ComputeGroundBounds(map_data);
  if (xz.min_x > xz.max_x) return;

  auto* camera_go = FindGameObject("MainCamera");
  if (!camera_go) return;
  auto* controller = camera_go->GetComponent<FreeCameraController>();
  if (!controller) return;

  const cfg::CameraConfig CAM;
  float ext = CAM.bounds_xz_extent;
  controller->SetBounds({
    {xz.min_x - ext, CAM.bounds_y_min, xz.min_z - ext},
    {xz.max_x + ext, CAM.bounds_y_max, xz.max_z + ext},
  });
}
