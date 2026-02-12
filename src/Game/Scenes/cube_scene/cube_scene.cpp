#include "cube_scene.h"

#include "Component/Renderer/mesh_renderer.h"
#include "Component/Renderer/particle_emitter.h"
#include "Component/camera_component.h"
#include "Component/transform_component.h"
#include "Debug/debug_drawer.h"
#include "Framework/Core/color.h"
#include "Framework/Input/input.h"
#include "Scripts/free_camera_controller.h"
#include "scene_id.h"
#include "scene_manager.h"

void CubeScene::OnEnter(AssetManager&) {
  SetupCamera();

  GetBackgroundSetting().SetClearColorValue(colors::ColorFromHex("#18181B"));

  cube_ = CreateGameObject("Cube", {.scale = {0.5f, 0.5f, 0.5f}});
  cube_->AddComponent<MeshRenderer>(MeshRenderer::Props{
    .mesh_type = DefaultMesh::Cube,
    .texture_path = "Content/textures/result_bg_1.png",
    .color = colors::White,
  });

  auto* particle_obj = CreateGameObject("Particles", {.position = {5, 3, 5}});
  auto* emitter = particle_obj->AddComponent<ParticleEmitter>(ParticleEmitter::Props{
    .texture_path = "Content/textures/sun_additive.png",
    .max_particles = 2000,
    .emit_rate = 500.0f,
    .particle_lifetime = 2.0f,
    .particle_size = {0.3f, 0.3f},
    .start_color = {1.0f, 0.6f, 0.1f, 1.0f},
    .end_color = {1.0f, 0.0f, 0.0f, 0.0f},
    .start_speed = 2.0f,
    .speed_variation = 1.5f,
    .gravity = {0, -2.0f, 0},
  });
  emitter->Play();

  auto* ring_obj = CreateGameObject("RingParticles", {.position = {-3, 0, -3}});
  auto* ring_emitter = ring_obj->AddComponent<ParticleEmitter>(ParticleEmitter::Props{
    .texture_path = "Content/textures/sun_additive.png",
    .max_particles = 2000,
    .emit_rate = 500.0f,
    .particle_lifetime = 1.0f,
    .particle_size = {0.1f, 0.1f},
    .start_color = {1.0f, 0.3f, 0.3f, 1.0f},
    .end_color = {0.1f, 0.5f, 1.0f, 0.0f},
    .start_speed = 1.0f,
    .speed_variation = 0.5f,
    .gravity = {0, 0.5f, 0},
    .spawn_fn = [](std::mt19937& rng) -> ParticleEmitter::SpawnParams {
      constexpr float RADIUS = 2.0f;  // position from center
      std::uniform_real_distribution<float> angle_dist(0.0f, Math::TwoPi);
      std::uniform_real_distribution<float> spread(-0.8f, 0.8f);
      float angle = angle_dist(rng);
      Vector3 on_circle = {std::cos(angle), 0.0f, std::sin(angle)};
      Vector3 up_dir = Vector3(spread(rng), 1.0f, spread(rng)).Normalized();

      return {.offset = on_circle * RADIUS, .direction = up_dir};
    },
  });
  ring_emitter->Play();
}

void CubeScene::OnPostUpdate(float dt) {
  rotation_angle_ += 30.0f * dt;
  cube_->GetComponent<TransformComponent>()->SetRotationEulerDegree({0.0f, rotation_angle_, 0.0f});

  auto* input = GetContext()->GetInput();
  if (input && input->GetKeyDown(Keyboard::KeyCode::F1)) {
    GetContext()->GetSceneManager()->RequestLoad(SceneId::TEST_SCENE);
  }
}

void CubeScene::OnExit() {
  cube_ = nullptr;
}

void CubeScene::SetupCamera() {
  auto* camera_obj = CreateGameObject("MainCamera", {.position = {0, 2, -8}});
  auto* camera = camera_obj->AddComponent<CameraComponent>();
  camera_obj->AddComponent<FreeCameraController>(FreeCameraController::Props{
    .movement_speed = 15.0f,
    .rotation_speed = 1.5f,
    .smoothness = 8.0f,
  });
  GetCameraSetting().Register(camera);
}

void CubeScene::OnDebugDraw(DebugDrawer& drawer) {
  DebugDrawer::GridConfig grid_config;
  grid_config.size = 20.0f;
  grid_config.cell_size = 1.0f;
  grid_config.y_level = 0.0f;
  grid_config.color = colors::Gray;
  drawer.DrawGrid(grid_config);

  drawer.DrawAxisGizmo();
}
