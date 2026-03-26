#pragma once

#include <vector>

#include "Component/behavior_component.h"
#include "PulseCB.generated.h"

class GameObject;

class PulsePathComponent : public BehaviorComponent<PulsePathComponent> {
 public:
  struct Props {
    float beam_thickness = 0.08f;
    float path_y_offset = 0.15f;
  };

  PulsePathComponent(GameObject* owner, const Props& props);

  void OnUpdate(float dt) override;
  void OnDestroy() override;

  void Show(GameObject* target, const PulseCB& params);
  void Hide();

 private:
  void RebuildSegments();
  void DestroySegments();

  Props props_;
  PulseCB shader_params_ = {};
  GameObject* target_ = nullptr;
  std::vector<GameObject*> segment_gos_;
  size_t last_waypoint_index_ = SIZE_MAX;
  size_t last_waypoint_count_ = 0;
};
