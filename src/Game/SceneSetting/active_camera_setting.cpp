#include "active_camera_setting.h"

#include <algorithm>

#include "Component/camera_component.h"
#include "game_object.h"

void ActiveCameraSetting::Register(CameraComponent* camera, int priority) {
  Unregister(camera);
  entries_.push_back({camera, priority});
}

void ActiveCameraSetting::Unregister(CameraComponent* camera) {
  std::erase_if(entries_, [camera](const Entry& e) { return e.camera == camera; });
}

CameraComponent* ActiveCameraSetting::GetActive() const {
  if (entries_.empty()) return nullptr;
  auto it = std::max_element(entries_.begin(), entries_.end(), [](const Entry& a, const Entry& b) { return a.priority < b.priority; });
  return it->camera;
}

void ActiveCameraSetting::Clear() {
  entries_.clear();
}

void ActiveCameraSetting::RemoveCamerasOwnedBy(GameObject* obj) {
  std::erase_if(entries_, [obj](const Entry& e) { return e.camera->GetOwner() == obj; });
}
