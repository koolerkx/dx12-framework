#include "active_ui_camera_setting.h"

#include <algorithm>

#include "Component/camera_component.h"
#include "game_object.h"

void ActiveUICameraSetting::Register(CameraComponent* camera, int priority) {
  Unregister(camera);
  entries_.push_back({camera, priority});
}

void ActiveUICameraSetting::Unregister(CameraComponent* camera) {
  std::erase_if(entries_, [camera](const Entry& e) { return e.camera == camera; });
}

CameraComponent* ActiveUICameraSetting::GetActive() const {
  if (entries_.empty()) return nullptr;
  auto it = std::max_element(entries_.begin(), entries_.end(), [](const Entry& a, const Entry& b) { return a.priority < b.priority; });
  return it->camera;
}

void ActiveUICameraSetting::Clear() {
  entries_.clear();
}

void ActiveUICameraSetting::RemoveCamerasOwnedBy(GameObject* obj) {
  std::erase_if(entries_, [obj](const Entry& e) { return e.camera->GetOwner() == obj; });
}
