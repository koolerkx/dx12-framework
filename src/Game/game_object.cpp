#include "game_object.h"

#include <cassert>

#include "Component/transform_component.h"
#include "Framework/Logging/logger.h"
#include "scene.h"

GameObject::GameObject(IScene* scene, const std::string& name) : name_(name), scene_(scene) {
  // CONSTRAINT: Every GameObject MUST have a TransformComponent.
  AddUniqueComponent<TransformComponent>();
}

GameObject::~GameObject() {
  if (is_started_) {
    OnDestroy();
  }
  for (auto& comp : components_) {
    if (comp->is_started_) {
      comp->OnDestroy();
    }
  }
}

GameContext* GameObject::GetContext() const {
  return scene_ ? scene_->GetContext() : nullptr;
}

bool GameObject::IsActiveInHierarchy() const {
  if (!active_) return false;
  return parent_ ? parent_->IsActiveInHierarchy() : true;
}

void GameObject::Init() {
  OnInit();
}

void GameObject::Start() {
  is_started_ = true;
  OnStart();
  FlushPendingStarts();
}

void GameObject::Update(float dt) {
  if (!active_) return;
  FlushPendingStarts();

  const size_t count = components_.size();
  for (size_t i = 0; i < count; ++i) {
    if (components_[i]->is_started_ && components_[i]->IsEnabled()) {
      components_[i]->OnUpdate(dt);
    }
  }
  for (auto* child : children_) {
    child->Update(dt);
  }
}

void GameObject::FixedUpdate(float dt) {
  if (!active_) return;
  const size_t count = components_.size();
  for (size_t i = 0; i < count; ++i) {
    if (components_[i]->is_started_ && components_[i]->IsEnabled()) {
      components_[i]->OnFixedUpdate(dt);
    }
  }
  for (auto* child : children_) {
    child->FixedUpdate(dt);
  }
}

void GameObject::DebugUpdate(float dt) {
  if (!active_) return;
  FlushPendingStarts();
  const size_t count = components_.size();
  for (size_t i = 0; i < count; ++i) {
    if (components_[i]->is_started_ && components_[i]->IsEnabled()) {
      components_[i]->OnDebugUpdate(dt);
    }
  }
  for (auto* child : children_) {
    child->DebugUpdate(dt);
  }
}

void GameObject::DebugFixedUpdate(float dt) {
  if (!active_) return;
  const size_t count = components_.size();
  for (size_t i = 0; i < count; ++i) {
    if (components_[i]->is_started_ && components_[i]->IsEnabled()) {
      components_[i]->OnDebugFixedUpdate(dt);
    }
  }
  for (auto* child : children_) {
    child->DebugFixedUpdate(dt);
  }
}

void GameObject::DebugDraw(DebugDrawer& drawer) {
  if (!active_) return;
  const size_t count = components_.size();
  for (size_t i = 0; i < count; ++i) {
    auto& comp = components_[i];
    if (comp->is_started_ && comp->IsEnabled() && comp->IsDebugDrawEnabled()) {
      comp->OnDebugDraw(drawer);
    }
  }
  for (auto* child : children_) {
    child->DebugDraw(drawer);
  }
}

void GameObject::FlushPendingStarts() {
  const size_t count = components_.size();
  for (size_t i = 0; i < count; ++i) {
    if (!components_[i]->is_started_) {
      components_[i]->is_started_ = true;
      components_[i]->OnReset();
      components_[i]->OnStart();
    }
  }
}

TransformComponent* GameObject::GetTransform() {
  auto transform = GetComponent<TransformComponent>();
  assert(transform && "Transform Component must exist.");
  return transform;
}

void GameObject::SetParent(GameObject* parent) {
  if (parent_ == parent) return;

  if (parent == this) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Game, Logger::Here(), "[GameObject] {}: Cannot set parent to self.", name_);
    return;
  }

  if (parent && IsDescendantOf(parent, this)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Game, Logger::Here(), "[GameObject] {}: circular parent hierarchy.", name_);
    return;
  }

  // Remove from old parent and replace with new
  if (parent_) {
    parent_->RemoveChild(this);
  }

  parent_ = parent;

  if (parent_) {
    parent_->AddChild(this);
  }

  for (auto& comp : components_) {
    comp->OnParentChanged();
  }
}

void GameObject::AddChild(GameObject* child) {
  children_.push_back(child);
}

void GameObject::RemoveChild(GameObject* child) {
  auto it = std::remove(children_.begin(), children_.end(), child);
  if (it != children_.end()) {
    children_.erase(it, children_.end());
  }
}

bool GameObject::IsDescendantOf(const GameObject* node, const GameObject* possible_ancestor) {
  if (!node || !possible_ancestor) return false;

  for (auto* p = node->GetParent(); p != nullptr; p = p->GetParent()) {
    if (p == possible_ancestor) return true;
  }
  return false;
}

void GameObject::DetachFromHierarchy() {
  if (parent_) {
    parent_->RemoveChild(this);
    parent_ = nullptr;
  }

  for (auto* child : children_) {
    if (child) {
      child->parent_ = nullptr;
    }
  }
  children_.clear();
}

void GameObject::Destroy() {
  is_pending_destroy_ = true;

  // Recursively mark all children for destruction
  for (auto* child : children_) {
    if (child) {
      child->Destroy();
    }
  }
}

bool GameObject::IsPendingDestroy() const {
  return is_pending_destroy_;
}

void GameObject::NotifyComponentAdded(IComponentBase* component) {
  if (scene_) {
    scene_->RegisterComponent(component);
  }
}

GameObject* GameObject::FindChild(const std::string& name) const {
  for (auto* child : children_) {
    if (child->GetName() == name) return child;
  }
  return nullptr;
}

GameObject* GameObject::FindChildRecursive(const std::string& name) const {
  for (auto* child : children_) {
    if (child->GetName() == name) return child;
    if (auto* found = child->FindChildRecursive(name)) return found;
  }
  return nullptr;
}

void GameObject::SetUUID(const framework::UUID& uuid) {
  if (scene_) {
    scene_->UnregisterGameObject(this);
  }
  uuid_ = uuid;
  if (scene_) {
    scene_->RegisterGameObject(this);
  }
}
