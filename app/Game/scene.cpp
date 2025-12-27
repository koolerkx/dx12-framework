// scene.cpp
#include "scene.h"

#include "game_object.h"

GameObject* IScene::CreateGameObject(const std::string& name) {
  auto obj = std::make_unique<GameObject>(name);
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

void IScene::Enter(Graphic& graphic) {
  OnEnter(graphic);
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
