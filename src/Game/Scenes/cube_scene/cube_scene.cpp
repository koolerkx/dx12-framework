#include "cube_scene.h"

#include "Asset/asset_manager.h"
#include "Component/Renderer/mesh_renderer.h"
#include "Component/camera_component.h"
#include "Component/transform_component.h"
#include "Framework/Core/color.h"
#include "Framework/Input/input.h"
#include "Framework/Math/Math.h"
#include "Scripts/free_camera_controller.h"
#include "scene_id.h"
#include "scene_manager.h"

void CubeScene::OnEnter(AssetManager& asset_manager) {
  texture_ = asset_manager.LoadTexture("Content/textures/result_bg_1.png");

  SetupCamera();

  cube_ = CreateGameObject("Cube", {.scale = {2, 2, 2}});
  cube_->AddComponent<MeshRenderer>(MeshRenderer::Props{
    .mesh = asset_manager.GetDefaultMesh(DefaultMesh::Cube),
    .texture = texture_.Get(),
    .color = colors::White,
  });
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
