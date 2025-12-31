#include "game.h"

#include "Scenes/test_scene/test_scene.h"
#include "game_context.h"

Game::Game() {
}

Game::~Game() {
  Shutdown();
}

void Game::Initialize() {
  if (context_ && context_->GetGraphic()) {
    asset_manager_.Initialize(context_->GetGraphic());
    context_->SetAssetManager(&asset_manager_);

    if (debug_drawer_.get() == nullptr) {
      debug_drawer_ = std::make_unique<DebugDrawer>(context_->GetGraphic());
    }
    context_->SetDebugDrawer(debug_drawer_.get());
  }

  current_scene_ = std::make_unique<TestScene>();
  if (current_scene_) {
    current_scene_->SetContext(context_);
    current_scene_->Enter(asset_manager_);
  }
}

void Game::Shutdown() {
  bool expected = false;
  if (!is_shutting_down_.compare_exchange_strong(expected, true)) {
    return;  // Already shutting down
  }

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
  if (!context_ || !context_->GetGraphic()) return;

  Graphic* graphic = context_->GetGraphic();
  frame_packet_.Clear();

  RenderFrameContext frame_context = graphic->BeginFrame();

  if (current_scene_) {
    current_scene_->Render(frame_packet_);
    current_scene_->OnRender(frame_packet_);
  }

  graphic->RenderScene(frame_context, frame_packet_);
  graphic->EndFrame(frame_context);
}
