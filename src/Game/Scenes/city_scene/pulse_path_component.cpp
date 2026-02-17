#include "pulse_path_component.h"

#include "Component/Renderer/mesh_renderer.h"
#include "Component/transform_component.h"
#include "Framework/Math/Math.h"
#include "Scenes/city_scene/object_movement_component.h"
#include "game_object.h"
#include "scene.h"

using Math::Quaternion;
using Math::Vector3;

PulsePathComponent::PulsePathComponent(GameObject* owner, const Props& props)
    : BehaviorComponent(owner), props_(props) {}

void PulsePathComponent::OnUpdate(float /*dt*/) {
  if (!target_) return;

  if (target_->IsPendingDestroy()) {
    Hide();
    return;
  }

  auto* movement = target_->GetComponent<ObjectMovementComponent>();
  if (!movement || !movement->IsMoving()) {
    DestroySegments();
    return;
  }

  size_t current_index = movement->GetCurrentWaypointIndex();
  size_t waypoint_count = movement->GetWaypoints().size();
  if (current_index != last_waypoint_index_ || waypoint_count != last_waypoint_count_) {
    RebuildSegments();
  }
}

void PulsePathComponent::OnDestroy() {
  DestroySegments();
}

void PulsePathComponent::Show(GameObject* target, const Graphics::PathPulseShader::Params& params) {
  DestroySegments();
  target_ = target;
  shader_params_ = params;
}

void PulsePathComponent::Hide() {
  DestroySegments();
  target_ = nullptr;
}

void PulsePathComponent::RebuildSegments() {
  DestroySegments();

  if (!target_) return;
  auto* movement = target_->GetComponent<ObjectMovementComponent>();
  if (!movement || !movement->IsMoving()) return;

  const auto& waypoints = movement->GetWaypoints();
  size_t start = movement->GetCurrentWaypointIndex();
  last_waypoint_index_ = start;
  last_waypoint_count_ = waypoints.size();

  if (start >= waypoints.size()) return;

  auto* scene = GetOwner()->GetScene();
  float accumulated_dist = 0.0f;
  static int segment_id = 0;

  for (size_t i = start; i + 1 < waypoints.size(); ++i) {
    Vector3 pos_a = {waypoints[i].x, props_.path_y_offset, waypoints[i].y};
    Vector3 pos_b = {waypoints[i + 1].x, props_.path_y_offset, waypoints[i + 1].y};

    Vector3 diff = pos_b - pos_a;
    float distance = diff.Length();
    if (distance < 0.001f) continue;

    Vector3 direction = diff / distance;
    Vector3 midpoint = (pos_a + pos_b) * 0.5f;
    Quaternion rotation = Quaternion::FromToRotation(Vector3::Up, direction);

    auto* go = scene->CreateGameObject("PulseSeg_" + std::to_string(segment_id++));
    go->SetTransient(true);
    go->SetParent(GetOwner());

    auto* transform = go->GetTransform();
    transform->SetPosition(midpoint);
    transform->SetRotation(rotation);
    transform->SetScale({props_.beam_thickness, distance, props_.beam_thickness});

    auto* renderer = go->AddComponent<MeshRenderer>(MeshRenderer::Props{
      .mesh_type = DefaultMesh::Cylinder,
    });

    auto params = shader_params_;
    params.distance_offset = accumulated_dist;
    params.segment_length = distance;
    renderer->SetShaderWithParams<Graphics::PathPulseShader>(params);

    segment_gos_.push_back(go);
    accumulated_dist += distance;
  }
}

void PulsePathComponent::DestroySegments() {
  for (auto* go : segment_gos_) {
    if (go) go->Destroy();
  }
  segment_gos_.clear();
  last_waypoint_index_ = SIZE_MAX;
  last_waypoint_count_ = 0;
}
