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

  cube_ = CreateGameObject("Cube");
  auto* transform = cube_->GetComponent<TransformComponent>();
  transform->SetScale({2.0f, 2.0f, 2.0f});

  auto* renderer = cube_->AddComponent<MeshRenderer>();
  renderer->SetMesh(asset_manager.GetDefaultMesh(DefaultMesh::Cube));
  renderer->SetTexture(texture_.Get());
  renderer->SetColor(colors::White);
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
  camera_object_ = nullptr;
}

void CubeScene::SetupCamera() {
  camera_object_ = CreateGameObject("MainCamera");
  auto* camera_transform = camera_object_->GetComponent<TransformComponent>();
  camera_transform->SetPosition({0.0f, 2.0f, -8.0f});

  auto* camera = camera_object_->AddComponent<CameraComponent>();
  camera->SetPerspective(Math::PiOver4, 16.0f / 9.0f, 0.1f, 1000.0f);

  auto* controller = camera_object_->AddComponent<FreeCameraController>();
  controller->SetMovementSpeed(15.0f);
  controller->SetRotationSpeed(1.5f);
  controller->SetSmoothness(8.0f);

  GetCameraSetting().Register(camera);
}
