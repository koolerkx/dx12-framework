#include "test_scene.h"

#include "Component/sprite_renderer.h"
#include "Component/transform_component.h"
#include "Graphic/graphic.h"
#include "character_mover_component.h"
#include "game_object.h"

void TestScene::OnEnter(Graphic& graphic) {
  auto& texture_manager = graphic.GetTextureManager();

  texture_background_ = texture_manager.LoadTexture(L"Content/textures/result_bg_1.png");
  texture_character_ = texture_manager.LoadTexture(L"Content/textures/ship_J.png");

  // Setup scene
  auto* background = CreateGameObject("Background");
  background->GetComponent<TransformComponent>()->SetPosition({500, 500, 0});
  auto* bg_sprite = background->AddComponent<SpriteRenderer>();
  bg_sprite->SetTexture(texture_background_.get());
  bg_sprite->SetSize({1920, 1080});

  character_object_ = CreateGameObject("Character");
  character_object_->SetParent(background);
  character_object_->GetComponent<TransformComponent>()->SetPosition({100, 50, 0});
  auto* char_sprite = character_object_->AddComponent<SpriteRenderer>();
  char_sprite->SetTexture(texture_character_.get());
  char_sprite->SetSize({150, 150});

  // Add custom component with game logic
  character_object_->AddComponent<CharacterMover>();
}

void TestScene::OnExit() {
  character_object_ = nullptr;
  texture_background_.reset();
  texture_character_.reset();
}
