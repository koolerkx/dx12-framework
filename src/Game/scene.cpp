// scene.cpp
#include "scene.h"

#include "Component/camera_component.h"
#include "Framework/Logging/logger.h"
#include "Graphic/graphic.h"
#include "game_object.h"

IScene::IScene() = default;
IScene::~IScene() = default;

GameObject* IScene::CreateGameObject(const std::string& name, const TransformComponent::Props& transform) {
  if (FindGameObject(name)) {
    Logger::LogFormat(LogLevel::Warn, LogCategory::Game, Logger::Here(), "GameObject with name '{}' already exists in scene", name);
  }

  auto obj = std::make_unique<GameObject>(this, name);
  GameObject* ptr = obj.get();
  game_objects_.push_back(std::move(obj));

  ptr->Init();
  ptr->GetTransform()->Apply(transform);

  return ptr;
}

void IScene::StartAllObjects() {
  FlushPendingStarts();
  is_started_ = true;
}

void IScene::FlushPendingStarts() {
  for (auto& obj : game_objects_) {
    if (!obj->IsStarted()) {
      obj->Start();
    }
  }
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
  for (auto& obj : game_objects_) {
    if (obj->GetParent() == nullptr) {
      obj->Update(dt);
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

void IScene::CleanupDestroyedObjects() {
  while (true) {
    size_t count_before = game_objects_.size();

    // Detach pending-destroy objects while all are still alive
    for (auto& obj : game_objects_) {
      if (obj->IsPendingDestroy()) {
        camera_setting_.RemoveCamerasOwnedBy(obj.get());
        ui_camera_setting_.RemoveCamerasOwnedBy(obj.get());
        obj->DetachFromHierarchy();
      }
    }

    auto it = std::remove_if(
      game_objects_.begin(), game_objects_.end(), [](const std::unique_ptr<GameObject>& obj) { return obj->IsPendingDestroy(); });
    game_objects_.erase(it, game_objects_.end());

    if (game_objects_.size() == count_before) break;
  }
}
