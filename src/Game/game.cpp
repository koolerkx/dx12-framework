#include "game.h"

#include "Debug/debug_drawer.h"
#include "Graphic/graphic.h"
#include "Scenes/cube_scene/cube_scene.h"
#include "Scenes/blank_scene.h"
#include "Scenes/empty_scene.h"
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

  context_->SetSceneManager(&scene_manager_);

  scene_manager_.Register<TestScene>(SceneId::TEST_SCENE);
  scene_manager_.Register<CubeScene>(SceneId::CUBE_SCENE);
  scene_manager_.Register<EmptyScene>(SceneId::EMPTY_SCENE);
  scene_manager_.Register<BlankScene>(SceneId::BLANK_SCENE);
  scene_manager_.RequestLoad(SceneId::TEST_SCENE);
  scene_manager_.ProcessPending(asset_manager_, context_, context_->GetGraphic());
}

void Game::Shutdown() {
  bool expected = false;
  if (!is_shutting_down_.compare_exchange_strong(expected, true)) {
    return;
  }

  scene_manager_.Shutdown(context_ ? context_->GetGraphic() : nullptr);
}

void Game::OnUpdate(float dt) {
  scene_manager_.ProcessPending(asset_manager_, context_, context_ ? context_->GetGraphic() : nullptr);
  IScene* scene = scene_manager_.GetCurrentScene();
  if (scene) {
    scene->Update(dt);
  }
}

void Game::OnFixedUpdate(float dt) {
  IScene* scene = scene_manager_.GetCurrentScene();
  if (scene) {
    scene->FixedUpdate(dt);
  }
}

void Game::OnRender() {
  if (!context_ || !context_->GetGraphic()) return;

  Graphic* graphic = context_->GetGraphic();
  frame_packet_.Clear();

  RenderFrameContext frame_context = graphic->BeginFrame();

  IScene* scene = scene_manager_.GetCurrentScene();
  if (scene) {
    scene->Render(frame_packet_);
    scene->OnRender(frame_packet_);

    if (debug_drawer_ && debug_drawer_->IsEnabled()) {
      scene->DebugDraw(*debug_drawer_);
    }
  }

  graphic->UploadPointLights(frame_context, frame_packet_);
  graphic->RenderScene(frame_context, frame_packet_);
  graphic->EndFrame(frame_context);
}
