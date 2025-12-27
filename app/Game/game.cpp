#include "game.h"

#include "Scenes/test_scene/test_scene.h"

Game::Game(Graphic& graphic) : graphic_(graphic) {
}

Game::~Game() {
  Shutdown();
}

void Game::Initialize() {
  asset_manager_.Initialize(&graphic_);

  current_scene_ = std::make_unique<TestScene>();
  if (current_scene_) {
    current_scene_->Enter(asset_manager_);
  }
}

void Game::Shutdown() {
  if (current_scene_) {
    current_scene_->Exit();
    current_scene_.reset();
  }
}

void Game::OnUpdate(float dt) {
  if (current_scene_) {
    current_scene_->Update(dt);
  }
}

void Game::OnFixedUpdate(float dt) {
  if (current_scene_) {
    current_scene_->FixedUpdate(dt);
  }
}

void Game::OnRender() {
  frame_packet_.Clear();

  RenderFrameContext frame_context = graphic_.BeginFrame();

  if (current_scene_) {
    current_scene_->Render(frame_packet_);
  }

  graphic_.RenderScene(frame_context, frame_packet_);
  graphic_.EndFrame(frame_context);
}
