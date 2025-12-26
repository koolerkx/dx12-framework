#include "game.h"

Game::Game(Graphic& graphic) : graphic_(graphic) {
  SpriteObject test_sprite;
  test_sprite.pos = {300.0f, 300.0f};
  test_sprite.size = {200.0f, 200.0f};
  test_sprite.tex = graphic_.myTexture[0].get();
  test_sprite.color = {1.0f, 1.0f, 1.0f, 1.0f};
  scene_.AddSprite(test_sprite);

  SpriteObject test_sprite2;
  test_sprite2.pos = {800.0f,300.0f};
  test_sprite2.size = {100.0f, 100.0f};
  test_sprite2.tex = graphic_.myTexture[1].get();
  test_sprite2.color = {1.0f, 1.0f, 1.0f, 0.5f};
  scene_.AddSprite(test_sprite2);

  
  SpriteObject test_sprite3;
  test_sprite3.pos = {1100.0f,300.0f};
  test_sprite3.size = {50.0f, 50.0f};
  test_sprite3.tex = graphic_.myTexture[0].get();
  test_sprite3.color = {1.0f, 1.0f, 1.0f, 1.0f};
  scene_.AddSprite(test_sprite3);
};

void Game::OnUpdate([[maybe_unused]] float dt) {
  RenderWorld world = scene_.BuildRenderWorld();

  auto frame = graphic_.BeginFrame();
  graphic_.RenderScene(frame, world);
  graphic_.EndFrame(frame);
}

void Game::OnFixedUpdate([[maybe_unused]] float dt) {
}
