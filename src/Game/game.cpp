#include "game.h"

#include "Debug/debug_drawer.h"
#include "Scenes/blank_scene.h"
#include "Scenes/city_scene/city_scene.h"
#include "Scenes/cube_scene/cube_scene.h"
#include "Scenes/empty_scene.h"
#include "Scenes/model_scene/model_scene.h"
#include "Scenes/test_scene/test_scene.h"
#include "Scenes/title_scene/title_scene.h"
#include "game_context.h"

Game::Game() {
}

Game::~Game() {
  Shutdown();
}

void Game::Initialize(const Props& props) {
  context_ = props.context;

  bool has_services = context_ && context_->GetTextureService() && context_->GetMeshService() && context_->GetFontService() &&
                      context_->GetDebugDrawService();

  if (has_services) {
    asset_manager_.Initialize({
      *context_->GetTextureService(),
      *context_->GetMeshService(),
      *context_->GetFontService(),
    });
    context_->SetAssetManager(&asset_manager_);

    if (debug_drawer_.get() == nullptr) {
      debug_drawer_ = std::make_unique<DebugDrawer>(context_->GetDebugDrawService());
    }
    debug_drawer_->SetEnabled(props.debug_draw_enabled);
    context_->SetDebugDrawer(debug_drawer_.get());
  }

  context_->SetSceneManager(&scene_manager_);
  context_->SetPlayStateSource(&play_state_);
  context_->SetSceneDefaults(props.scene_defaults);

  if (props.auto_play) {
    play_state_ = PlayState::Playing;
  }

  scene_manager_.Register<TestScene>(SceneId::TEST_SCENE);
  scene_manager_.Register<CubeScene>(SceneId::CUBE_SCENE);
  scene_manager_.Register<EmptyScene>(SceneId::EMPTY_SCENE);
  scene_manager_.Register<BlankScene>(SceneId::BLANK_SCENE);
  scene_manager_.Register<ModelScene>(SceneId::MODEL_SCENE);
  scene_manager_.Register<CityScene>(SceneId::CITY_SCENE);
  scene_manager_.Register<TitleScene>(SceneId::TITLE_SCENE);
  scene_manager_.RequestLoad(props.initial_scene);
  scene_manager_.ProcessPending(asset_manager_, context_, context_->GetRenderService());
}

void Game::Shutdown() {
  bool expected = false;
  if (!is_shutting_down_.compare_exchange_strong(expected, true)) {
    return;
  }

  scene_manager_.Shutdown(context_ ? context_->GetRenderService() : nullptr);
}

void Game::Play() {
  if (play_state_ == PlayState::Playing) return;
  play_state_ = PlayState::Playing;
}

void Game::Pause() {
  if (play_state_ != PlayState::Playing) return;
  play_state_ = PlayState::Paused;
}

void Game::Stop() {
  if (play_state_ == PlayState::Stopped) return;
  IScene* scene = scene_manager_.GetCurrentScene();
  if (scene) scene->ResetAllComponents();
  play_state_ = PlayState::Stopped;
}

void Game::OnUpdate(float dt) {
  if (play_state_ == PlayState::Playing) elapsed_time_ += dt;
  scene_manager_.ProcessPending(asset_manager_, context_, context_ ? context_->GetRenderService() : nullptr);
  IScene* scene = scene_manager_.GetCurrentScene();
  if (!scene) return;

  if (play_state_ == PlayState::Playing) {
    scene->Update(dt);
  } else {
    scene->DebugUpdate(dt);
  }
}

void Game::OnFixedUpdate(float dt) {
  IScene* scene = scene_manager_.GetCurrentScene();
  if (!scene) return;
  if (play_state_ == PlayState::Playing) {
    scene->FixedUpdate(dt);
  } else {
    scene->DebugFixedUpdate(dt);
  }
}

void Game::CollectRenderData(FramePacket& packet) {
  packet.Clear();
  packet.time = elapsed_time_;

  IScene* scene = scene_manager_.GetCurrentScene();
  if (!scene) return;

  scene->Render(packet);
  scene->OnRender(packet);

  if (debug_drawer_ && debug_drawer_->IsEnabled()) {
    scene->DebugDraw(*debug_drawer_);
  }
}
