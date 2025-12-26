#include "game.h"

Game::Game(Graphic& graphic) : graphic_(graphic) {
  SpriteObject test_sprite;
  test_sprite.pos = {300.0f, 300.0f};
  test_sprite.size = {200.0f, 200.0f};
  test_sprite.tex = graphic_.myTexture[0].get();
  test_sprite.color = {1.0f, 1.0f, 1.0f, 1.0f};
  scene_.AddSprite(test_sprite);
};

void Game::OnUpdate([[maybe_unused]] float dt) {
  RenderWorld world = scene_.BuildRenderWorld();

  auto frame = graphic_.BeginFrame();
  graphic_.RenderScene(frame, world);
  graphic_.EndFrame(frame);
}

void Game::OnFixedUpdate([[maybe_unused]] float dt) {
}
