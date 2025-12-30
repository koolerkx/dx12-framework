#include "test_scene.h"

#include <DirectXMath.h>

#include "Asset/asset_manager.h"
#include "Component/free_camera_controller.h"
#include "Component/mesh_renderer.h"
#include "Component/sprite_renderer.h"
#include "Component/transform_component.h"
#include "character_mover_component.h"
#include "test_scene.h"

static float rotation_angle_ = 0.0f;
static float rotation_speed_ = 15.0f;

void TestScene::OnEnter(AssetManager& asset_manager) {
  texture_background_ = asset_manager.LoadTexture("Content/textures/result_bg_1.png");
  texture_character_ = asset_manager.LoadTexture("Content/textures/ship_J.png");

  SetupCamera();

  cube_object_ = CreateGameObject("Cube");
  auto* cube_transform = cube_object_->GetComponent<TransformComponent>();
  cube_transform->SetPosition({0.0f, 0.0f, 0.0f});  // At origin
  cube_transform->SetScale({2.0f, 2.0f, 2.0f});     // Scale to 2x2x2

  auto* mesh_renderer = cube_object_->AddComponent<MeshRenderer>();
  mesh_renderer->SetMesh(asset_manager.GetDefaultMesh(DefaultMesh::Cube));
  mesh_renderer->SetTexture(texture_background_.Get());  // Use ship texture for testing
  mesh_renderer->SetColor({1.0f, 1.0f, 1.0f, 1.0f});

  // auto* background = CreateGameObject("Background");
  // background->GetComponent<TransformComponent>()->SetPosition({500, 500, 0});
  // auto* bg_sprite = background->AddComponent<SpriteRenderer>();
  // bg_sprite->SetTexture(texture_background_.Get());
  // bg_sprite->SetSize({1920, 1080});

  character_object_ = CreateGameObject("Character");
  // character_object_->SetParent(background);
  character_object_->GetComponent<TransformComponent>()->SetPosition({500, 500, 0});
  auto* char_sprite = character_object_->AddComponent<SpriteRenderer>();
  char_sprite->SetTexture(texture_character_.Get());
  char_sprite->SetSize({150, 150});

  character_object_->AddComponent<CharacterMover>();
}

void TestScene::OnPostUpdate(float dt) {
  rotation_angle_ += rotation_speed_ * dt;

  cube_object_->GetComponent<TransformComponent>()->SetRotationEulerDegree({rotation_angle_, rotation_angle_, 0.0f});
}

void TestScene::OnExit() {
  character_object_ = nullptr;
}

void TestScene::SetupCamera() {
  camera_object_ = CreateGameObject("MainCamera");
  auto* camera_transform = camera_object_->GetComponent<TransformComponent>();
  camera_transform->SetPosition({0.0f, 0.0f, -10.0f});  // Move camera back

  auto* camera = camera_object_->AddComponent<CameraComponent>();
  camera->SetPerspective(DirectX::XM_PIDIV4, 16.0f / 9.0f, 0.1f, 1000.0f);

  auto* controller = camera_object_->AddComponent<FreeCameraController>();
  controller->SetMovementSpeed(15.0f);
  controller->SetRotationSpeed(1.5f);
  controller->SetSmoothness(8.0f);

  SetActiveCamera(camera);
}
