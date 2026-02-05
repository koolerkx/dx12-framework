// scene.cpp
#include "scene.h"

#include "Framework/Logging/logger.h"
#include "game_object.h"

IScene::IScene() = default;
IScene::~IScene() = default;

GameObject* IScene::CreateGameObject(const std::string& name) {
  auto obj = std::make_unique<GameObject>(this, name);
  GameObject* ptr = obj.get();
  game_objects_.push_back(std::move(obj));

  if (is_started_) {
    ptr->Start();
  }

  return ptr;
}

void IScene::StartAllObjects() {
  for (auto& obj : game_objects_) {
    obj->Start();
  }
  is_started_ = true;
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
  active_camera_ = nullptr;

  game_objects_.clear();
  is_started_ = false;
}

void IScene::Update(float dt) {
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
  if (active_camera_) {
    packet.main_camera = active_camera_->GetCameraData();
  } else {
    Logger::LogFormat(LogLevel::Warn, LogCategory::Game, Logger::Here(), "[IScene::Render] No active camera, using default setting");

    // Fallback: default camera
    using namespace DirectX;
    CameraData default_camera;
    StoreMatrixToCameraData(default_camera, XMMatrixIdentity(), XMMatrixPerspectiveFovLH(XM_PIDIV4, 16.0f / 9.0f, 0.1f, 1000.0f));

    default_camera.position = XMFLOAT3(0, 0, -5);
    default_camera.forward = XMFLOAT3(0, 0, 1);
    default_camera.up = XMFLOAT3(0, 1, 0);
    packet.main_camera = default_camera;
  }

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

void IScene::DestroyGameObject(GameObject* obj) {
  if (!obj) return;
  obj->Destroy();
}

void IScene::CleanupDestroyedObjects() {
  while (true) {
    size_t count_before = game_objects_.size();

    // Detach pending-destroy objects while all are still alive
    for (auto& obj : game_objects_) {
      if (obj->IsPendingDestroy()) {
        if (active_camera_ && active_camera_->GetOwner() == obj.get()) {
          active_camera_ = nullptr;
        }
        obj->DetachFromHierarchy();
      }
    }

    auto it = std::remove_if(
      game_objects_.begin(), game_objects_.end(), [](const std::unique_ptr<GameObject>& obj) { return obj->IsPendingDestroy(); });
    game_objects_.erase(it, game_objects_.end());

    if (game_objects_.size() == count_before) break;
  }
}
