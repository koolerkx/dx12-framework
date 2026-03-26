#include "turret_component.h"

#include "Component/Renderer/mesh_renderer.h"
#include "Component/transform_component.h"
#include "Framework/Math/Math.h"
#include "Framework/Shader/shader_descriptor.h"
#include "Framework/Shader/shader_id.h"
#include "Framework/Shader/shader_registration.h"
#include "Framework/Shader/vertex_shaders.h"
#include "LaserCB.generated.h"
#include "game_object.h"
#include "scene.h"
#include "scene_events.h"

using Math::Quaternion;
using Math::Vector3;

void TurretComponent::OnStart() {
  auto* bus = GetContext()->GetEventBus().get();
  event_scope_.Subscribe<GameOverEvent>(*bus, [this](const GameOverEvent&) {
    is_running_ = false;
    DestroyLaser();
  });
}

void TurretComponent::OnUpdate(float /*dt*/) {
  if (!is_running_) return;

  if (has_laser_target_ && ShouldShowLaser()) {
    UpdateLaser();
  } else {
    DestroyLaser();
  }
}

bool TurretComponent::ShouldShowLaser() const {
  return laser_visibility_ == LaserVisibility::AlwaysInRange || highlighted_;
}

void TurretComponent::OnDestroy() {
  DestroyLaser();
}

void TurretComponent::UpdateLaser() {
  constexpr float BEAM_THICKNESS = 0.05f;

  auto tower_pos = GetOwner()->GetTransform()->GetWorldPosition();
  auto enemy_pos = laser_target_position_;

  auto direction = enemy_pos - tower_pos;
  float distance = direction.Length();
  if (distance < 0.001f) {
    DestroyLaser();
    return;
  }

  direction = direction / distance;

  if (!laser_go_) {
    static int laser_id = 0;
    auto* scene = GetOwner()->GetScene();
    laser_go_ = scene->CreateGameObject("Laser_" + std::to_string(laser_id++), {});
    laser_go_->SetTransient(true);

    if (auto* reg = GetContext()->GetShaderRegistration()) {
      ShaderDescriptor desc{
        .id = HashShaderName("LaserBeam"),
        .name = "LaserBeam",
        .vs_path = VS::Basic3D::PATH,
        .ps_path = L"Content/shaders/laser_beam.ps.cso",
        .vertex_format = VS::Basic3D::VERTEX_FORMAT,
        .default_settings =
          {
            .blend_mode = Rendering::BlendMode::Additive,
            .depth_test = true,
            .double_sided = true,
            .render_target_format = Rendering::RenderTargetFormat::HDR,
          },
      };
      reg->RegisterShader(desc);
    }

    auto* renderer = laser_go_->AddComponent<MeshRenderer>(MeshRenderer::Props{
      .mesh_type = DefaultMesh::Cylinder,
    });
    renderer->SetShaderId(HashShaderName("LaserBeam"));
    renderer->SetRenderLayer(RenderLayer::Transparent);
    renderer->SetRenderSettings({
      .blend_mode = Rendering::BlendMode::Additive,
      .depth_test = true,
      .double_sided = true,
      .render_target_format = Rendering::RenderTargetFormat::HDR,
    });
    LaserCB laser_params{};
    laser_params.laserColor = {1.0f, 0.2f, 0.2f};
    laser_params.emissiveIntensity = 10.0f;
    laser_params.pulseSpeed = 15.0f;
    laser_params.pulseFrequency = 20.0f;
    laser_params.beamWidth = 2.0f;
    laser_params.endFadeRatio = 0.4f;
    laser_params.endFadePower = 3.0f;
    renderer->SetCustomData(laser_params);
  }

  auto midpoint = (tower_pos + enemy_pos) * 0.5f;
  auto rotation = Quaternion::FromToRotation(Vector3::Up, direction);

  auto* transform = laser_go_->GetTransform();
  transform->SetPosition(midpoint);
  transform->SetRotation(rotation);
  transform->SetScale({BEAM_THICKNESS, distance, BEAM_THICKNESS});
}

void TurretComponent::DestroyLaser() {
  if (laser_go_) {
    laser_go_->Destroy();
    laser_go_ = nullptr;
  }
}
