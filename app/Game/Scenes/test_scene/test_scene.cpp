#include "test_scene.h"

#include "Asset/asset_manager.h"
#include "Component/sprite_renderer.h"
#include "Component/transform_component.h"
#include "character_mover_component.h"
#include "test_scene.h"

void TestScene::OnEnter(AssetManager& asset_manager) {
  texture_background_ = asset_manager.LoadTexture("Content/textures/result_bg_1.png");
  texture_character_ = asset_manager.LoadTexture("Content/textures/ship_J.png");

  SetupCamera();

  auto* background = CreateGameObject("Background");
  background->GetComponent<TransformComponent>()->SetPosition({500, 500, 0});
  auto* bg_sprite = background->AddComponent<SpriteRenderer>();
  bg_sprite->SetTexture(texture_background_.Get());
  bg_sprite->SetSize({1920, 1080});

  character_object_ = CreateGameObject("Character");
  character_object_->SetParent(background);
  character_object_->GetComponent<TransformComponent>()->SetPosition({100, 50, 0});
  auto* char_sprite = character_object_->AddComponent<SpriteRenderer>();
  char_sprite->SetTexture(texture_character_.Get());
  char_sprite->SetSize({150, 150});

  character_object_->AddComponent<CharacterMover>();
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

  SetActiveCamera(camera);
}
