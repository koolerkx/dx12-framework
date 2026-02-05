#pragma once
#include <vector>

class CameraComponent;
class GameObject;

class ActiveUICameraSetting {
 public:
  void Register(CameraComponent* camera, int priority = 0);
  void Unregister(CameraComponent* camera);
  CameraComponent* GetActive() const;
  void Clear();
  void RemoveCamerasOwnedBy(GameObject* obj);

 private:
  struct Entry {
    CameraComponent* camera;
    int priority;
  };
  std::vector<Entry> entries_;
};
