// scene.cpp
#include "scene.h"

#include <iostream>

#include "game_object.h"


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
  game_objects_.clear();
  is_started_ = false;
}

void IScene::Update(float dt) {
  OnPreUpdate(dt);
  UpdateRootObjects(dt);
  OnPostUpdate(dt);
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
    std::cerr << "[IScene::Render] No active camera, using default setting" << std::endl;

    // Fallback: default camera
    using namespace DirectX;
    CameraData default_camera;
    XMStoreFloat4x4(&default_camera.view, XMMatrixIdentity());
    XMStoreFloat4x4(&default_camera.proj, XMMatrixPerspectiveFovLH(XM_PIDIV4, 16.0f / 9.0f, 0.1f, 1000.0f));
    default_camera.position = XMFLOAT3(0, 0, -5);
    default_camera.forward = XMFLOAT3(0, 0, 1);
    default_camera.up = XMFLOAT3(0, 1, 0);
    packet.main_camera = default_camera;
  }

  RenderRootObjects(packet);
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
