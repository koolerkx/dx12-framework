#pragma once

#include <vector>

#include "Component/behavior_component.h"

class GameObject;

struct PathPulseParams {
  float pulse_r, pulse_g, pulse_b;
  float emissive_intensity = 8.0f;
  float pulse_speed = 4.0f;
  float pulse_frequency = 10.0f;
  float pulse_width = 1.0f;
  float distance_offset = 0;
  float segment_length = 0;
  float _pad[3] = {0, 0, 0};
};
static_assert(sizeof(PathPulseParams) == 48);

class PulsePathComponent : public BehaviorComponent<PulsePathComponent> {
 public:
  struct Props {
    float beam_thickness = 0.08f;
    float path_y_offset = 0.15f;
  };

  PulsePathComponent(GameObject* owner, const Props& props);

  void OnUpdate(float dt) override;
  void OnDestroy() override;

  void Show(GameObject* target, const PathPulseParams& params);
  void Hide();

 private:
  void RebuildSegments();
  void DestroySegments();

  Props props_;
  PathPulseParams shader_params_ = {};
  GameObject* target_ = nullptr;
  std::vector<GameObject*> segment_gos_;
  size_t last_waypoint_index_ = SIZE_MAX;
  size_t last_waypoint_count_ = 0;
};
