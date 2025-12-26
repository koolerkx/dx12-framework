#include "scene.h"

#include "Graphic/graphic.h"

void TestScene::OnEnter(Graphic& graphic) {
  auto& tetxure_manager = graphic.GetTextureManager();
  texture_background_ = tetxure_manager.LoadTexture(L"Content/textures/result_bg_1.png");
  texture_character_ = tetxure_manager.LoadTexture(L"Content/textures/ship_J.png");

  character_pos_ = {100.0f, 100.0f};
}

void TestScene::OnUpdate(float dt) {
  static float time_acc = 0.0f;
  time_acc += dt;

  character_pos_.x = 400.0f + std::cos(time_acc) * 100.0f;
  character_pos_.y = 300.0f + std::sin(time_acc) * 100.0f;
}

void TestScene::OnRender(RenderWorld& world) {
  // Clear previous frame data (if RenderWorld is persistent, otherwise it's empty)
  // world.ui_sprites.clear(); // If the vector isn't cleared by the renderer

  if (texture_background_) {
    UiSpriteItem bg_item;
    bg_item.pos = {0.0f, 0.0f};
    bg_item.size = {1920.0f, 1080.0f};
    bg_item.tex = reinterpret_cast<TextureHandle>(texture_background_.get());
    bg_item.color = {1.0f, 1.0f, 1.0f, 1.0f};
    world.ui_sprites.push_back(bg_item);
  }

  if (texture_character_) {
    UiSpriteItem char_item;
    char_item.pos = character_pos_;
    char_item.size = {128.0f, 128.0f};
    char_item.tex = reinterpret_cast<TextureHandle>(texture_character_.get());
    char_item.color = {1.0f, 1.0f, 1.0f, 1.0f};
    world.ui_sprites.push_back(char_item);
  }
}

void TestScene::OnExit() {
  texture_background_.reset();
  texture_character_.reset();
}
