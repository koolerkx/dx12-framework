#include "test_scene.h"

#include "Asset/asset_manager.h"
#include "Component/sprite_renderer.h"
#include "Component/transform_component.h"
#include "character_mover_component.h"
#include "test_scene.h"

void TestScene::OnEnter(AssetManager& asset_manager) {
  texture_background_ = asset_manager.LoadTexture("Content/textures/result_bg_1.png");
  texture_character_ = asset_manager.LoadTexture("Content/textures/ship_J.png");

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
