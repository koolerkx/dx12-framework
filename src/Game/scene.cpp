// scene.cpp
#include "scene.h"

#include "Component/camera_component.h"
#include "Framework/Logging/logger.h"
#include "Graphic/graphic.h"
#include "game_object.h"

IScene::IScene() = default;
IScene::~IScene() = default;

void IScene::ClearAllObjects() {
  for (auto& obj : game_objects_) {
    obj->DetachFromHierarchy();
  }
  camera_setting_.Clear();
  ui_camera_setting_.Clear();
  gameobject_uuid_map_.clear();
  component_uuid_map_.clear();
  game_objects_.clear();
}

GameObject* IScene::CreateGameObject(const std::string& name, const TransformComponent::Props& transform) {
  if (FindGameObject(name)) {
    Logger::LogFormat(LogLevel::Warn, LogCategory::Game, Logger::Here(), "GameObject with name '{}' already exists in scene", name);
  }

  auto obj = std::make_unique<GameObject>(this, name);
  GameObject* ptr = obj.get();
  game_objects_.push_back(std::move(obj));
  RegisterGameObject(ptr);

  ptr->Init();
  ptr->GetTransform()->Apply(transform);

  return ptr;
}

void IScene::StartAllObjects() {
  FlushPendingStarts();
  is_started_ = true;
}

void IScene::FlushPendingStarts() {
  // Index-based loop: OnStart may create new GameObjects (e.g. ModelComponent),
  // which appends to game_objects_ and would invalidate range-for iterators.
  for (size_t i = 0; i < game_objects_.size(); ++i) {
    if (!game_objects_[i]->IsStarted()) {
      game_objects_[i]->Start();
    }
  }
}

void IScene::ApplyDefaults(const SceneDefaults& defaults) {
  light_setting_.SetAzimuth(defaults.light_azimuth);
  light_setting_.SetElevation(defaults.light_elevation);
  light_setting_.SetIntensity(defaults.light_intensity);
  light_setting_.SetDirectionalColor({defaults.light_color[0], defaults.light_color[1], defaults.light_color[2]});
  light_setting_.SetAmbientIntensity(defaults.ambient_intensity);
  light_setting_.SetAmbientColor({defaults.ambient_color[0], defaults.ambient_color[1], defaults.ambient_color[2]});

  shadow_setting_.SetEnabled(defaults.shadow_enabled);
  shadow_setting_.SetResolution(defaults.shadow_resolution);
  shadow_setting_.SetCascadeCount(defaults.shadow_cascade_count);
  shadow_setting_.SetAlgorithm(defaults.shadow_algorithm);
  shadow_setting_.SetShadowDistance(defaults.shadow_distance);
  shadow_setting_.SetLightDistance(defaults.light_distance);
  shadow_setting_.SetCascadeBlendRange(defaults.cascade_blend_range);
  shadow_setting_.SetShadowColor({defaults.shadow_color[0], defaults.shadow_color[1], defaults.shadow_color[2]});
  shadow_setting_.SetLightSize(defaults.light_size);

  background_setting_.SetMode(defaults.background_mode);
  background_setting_.SetClearColorValue(
    {defaults.clear_color[0], defaults.clear_color[1], defaults.clear_color[2], defaults.clear_color[3]});
}

void IScene::Enter(AssetManager& asset_manager) {
  OnEnter(asset_manager);
  StartAllObjects();
}

void IScene::Exit() {
  OnExit();

  for (auto& obj : game_objects_) {
    obj->DetachFromHierarchy();
  }
  camera_setting_.Clear();
  ui_camera_setting_.Clear();

  gameobject_uuid_map_.clear();
  component_uuid_map_.clear();
  game_objects_.clear();
  is_started_ = false;
}

void IScene::Update(float dt) {
  FlushPendingStarts();
  OnPreUpdate(dt);
  UpdateRootObjects(dt);
  OnPostUpdate(dt);
  CleanupDestroyedObjects();
}

void IScene::FixedUpdate(float dt) {
  OnPreFixedUpdate(dt);
  FixedUpdateRootObjects(dt);
  OnPostFixedUpdate(dt);
}

void IScene::DebugUpdate(float dt) {
  FlushPendingStarts();
  OnDebugUpdate(dt);
  DebugUpdateRootObjects(dt);
  CleanupDestroyedObjects();
}

void IScene::DebugFixedUpdate(float dt) {
  OnDebugFixedUpdate(dt);
  DebugFixedUpdateRootObjects(dt);
}

void IScene::DebugUpdateRootObjects(float dt) {
  for (size_t i = 0; i < game_objects_.size(); ++i) {
    if (game_objects_[i]->GetParent() == nullptr) {
      game_objects_[i]->DebugUpdate(dt);
    }
  }
}

void IScene::DebugFixedUpdateRootObjects(float dt) {
  for (auto& obj : game_objects_) {
    if (obj->GetParent() == nullptr) {
      obj->DebugFixedUpdate(dt);
    }
  }
}

void IScene::Render(FramePacket& packet) {
  auto* active_camera = camera_setting_.GetActive();
  if (active_camera) {
    packet.main_camera = active_camera->GetCameraData();
  } else {
    Logger::LogFormat(LogLevel::Warn, LogCategory::Game, Logger::Here(), "[IScene::Render] No active camera, using default setting");

    CameraData default_camera;
    StoreMatrixToCameraData(
      default_camera, Math::Matrix4::Identity, Math::Matrix4::CreatePerspectiveFOV(Math::PiOver4, 16.0f / 9.0f, 0.1f, 1000.0f));

    default_camera.position = Math::Vector3(0, 0, -5);
    default_camera.forward = Math::Vector3::Forward;
    default_camera.up = Math::Vector3::Up;
    packet.main_camera = default_camera;
  }

  auto* ui_camera = ui_camera_setting_.GetActive();
  if (ui_camera) {
    packet.ui_camera = ui_camera->GetCameraData();
  } else if (context_ && context_->GetGraphic()) {
    auto* gfx = context_->GetGraphic();
    packet.ui_camera =
      MakeScreenSpaceCamera(static_cast<float>(gfx->GetFrameBufferWidth()), static_cast<float>(gfx->GetFrameBufferHeight()));
  }

  packet.background = background_setting_.ToConfig();
  packet.lighting = light_setting_.ToConfig();
  packet.shadow = shadow_setting_.ToConfig();

  RenderRootObjects(packet);

  // Call derived class custom rendering (e.g., debug visualization)
  OnRender(packet);
}

void IScene::UpdateRootObjects(float dt) {
  // Index-based loop: OnUpdate may create new GameObjects (e.g. transient visual effects),
  // which appends to game_objects_ and would invalidate range-for iterators.
  for (size_t i = 0; i < game_objects_.size(); ++i) {
    if (game_objects_[i]->GetParent() == nullptr) {
      game_objects_[i]->Update(dt);
    }
  }
}

void IScene::FixedUpdateRootObjects(float dt) {
  for (auto& obj : game_objects_) {
    if (obj->GetParent() == nullptr) {
      obj->FixedUpdate(dt);
    }
  }
}

void IScene::RenderRootObjects(FramePacket& packet) {
  for (auto& obj : game_objects_) {
    if (obj->GetParent() == nullptr) {
      obj->Render(packet);
    }
  }
}

void IScene::DebugDraw(DebugDrawer& drawer) {
  DebugDrawRootObjects(drawer);
  OnDebugDraw(drawer);
}

void IScene::DebugDrawRootObjects(DebugDrawer& drawer) {
  for (auto& obj : game_objects_) {
    if (obj->GetParent() == nullptr) {
      obj->DebugDraw(drawer);
    }
  }
}

GameObject* IScene::FindGameObject(const std::string& name) const {
  for (auto& obj : game_objects_) {
    if (obj->GetName() == name) return obj.get();
  }
  return nullptr;
}

void IScene::DestroyGameObject(GameObject* obj) {
  if (!obj) return;
  obj->Destroy();
}

void IScene::DestroyGameObject(const std::string& name) {
  if (auto* obj = FindGameObject(name)) {
    obj->Destroy();
  }
}

void IScene::RegisterGameObject(GameObject* obj) {
  gameobject_uuid_map_[obj->GetUUID()] = obj;
}

void IScene::UnregisterGameObject(GameObject* obj) {
  gameobject_uuid_map_.erase(obj->GetUUID());
}

void IScene::RegisterComponent(IComponentBase* component) {
  component_uuid_map_[component->GetUUID()] = component;
}

void IScene::UnregisterGameObjectAndComponents(GameObject* obj) {
  gameobject_uuid_map_.erase(obj->GetUUID());
  for (const auto& comp : obj->GetComponents()) {
    component_uuid_map_.erase(comp->GetUUID());
  }
}

GameObject* IScene::FindGameObjectByUUID(const framework::UUID& uuid) const {
  auto it = gameobject_uuid_map_.find(uuid);
  return it != gameobject_uuid_map_.end() ? it->second : nullptr;
}

IComponentBase* IScene::FindComponentByUUID(const framework::UUID& uuid) const {
  auto it = component_uuid_map_.find(uuid);
  return it != component_uuid_map_.end() ? it->second : nullptr;
}

void IScene::ProcessPendingLifecycle() {
  FlushPendingStarts();
  CleanupDestroyedObjects();
}

void IScene::ResetAllComponents() {
  for (auto& obj : game_objects_) {
    for (auto& comp : obj->GetComponents()) {
      if (comp->IsStarted()) comp->OnReset();
    }
  }
}

void IScene::CleanupDestroyedObjects() {
  while (true) {
    size_t count_before = game_objects_.size();

    // Detach pending-destroy objects while all are still alive
    for (auto& obj : game_objects_) {
      if (obj->IsPendingDestroy()) {
        camera_setting_.RemoveCamerasOwnedBy(obj.get());
        ui_camera_setting_.RemoveCamerasOwnedBy(obj.get());
        UnregisterGameObjectAndComponents(obj.get());
        obj->DetachFromHierarchy();
      }
    }

    auto it = std::remove_if(
      game_objects_.begin(), game_objects_.end(), [](const std::unique_ptr<GameObject>& obj) { return obj->IsPendingDestroy(); });
    game_objects_.erase(it, game_objects_.end());

    if (game_objects_.size() == count_before) break;
  }
}
