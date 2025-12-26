#include "game.h"

Game::Game(Graphic& graphic) : graphic_(graphic) {
}

Game::~Game() {
  if (current_scene_) {
    current_scene_->OnExit();
  }
}

void Game::Initialize() {
  current_scene_ = std::make_unique<TestScene>();

  if (current_scene_) {
    current_scene_->OnEnter(graphic_);
  }
}

void Game::OnUpdate(float dt) {
  if (current_scene_) {
    current_scene_->OnUpdate(dt);
  }
}

void Game::OnFixedUpdate(float) {
}

void Game::OnRender() {
  render_world_.ui_sprites.clear();

  RenderFrameContext frame_context = graphic_.BeginFrame();

  if (current_scene_) {
    current_scene_->OnRender(render_world_);
  }

  graphic_.RenderScene(frame_context, render_world_);

  graphic_.EndFrame(frame_context);
}
