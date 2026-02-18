#include "SceneSetting/scene_transition_overlay.h"

#include <algorithm>

#include "Component/Renderer/ui_sprite_renderer.h"
#include "game_object.h"
#include "scene.h"

void SceneTransitionOverlay::Create(IScene* scene, const std::string& name) {
  overlay_go_ = scene->CreateGameObject(name);
  sprite_ = overlay_go_->AddComponent<UISpriteRenderer>(UISpriteRenderer::Props{
    .texture_path = "Content/textures/white.png",
    .color = {0.0f, 0.0f, 0.0f, 0.0f},
    .layer_id = 0,
  });
  overlay_go_->SetActive(false);
}

void SceneTransitionOverlay::FadeIn(std::function<void()> on_complete) {
  target_opacity_ = 1.0f;
  on_complete_ = std::move(on_complete);
  overlay_go_->SetActive(true);
}

void SceneTransitionOverlay::FadeOut(std::function<void()> on_complete) {
  target_opacity_ = 0.0f;
  on_complete_ = std::move(on_complete);
  overlay_go_->SetActive(true);
}

void SceneTransitionOverlay::SetOpaque() {
  opacity_ = 1.0f;
  target_opacity_ = 1.0f;
  on_complete_ = nullptr;
  sprite_->SetColor({0.0f, 0.0f, 0.0f, 1.0f});
  overlay_go_->SetActive(true);
}

void SceneTransitionOverlay::SetTransparent() {
  opacity_ = 0.0f;
  target_opacity_ = 0.0f;
  on_complete_ = nullptr;
  sprite_->SetColor({0.0f, 0.0f, 0.0f, 0.0f});
  overlay_go_->SetActive(false);
}

void SceneTransitionOverlay::Update(float dt) {
  if (opacity_ == target_opacity_) return;

  float step = FADE_SPEED * dt;
  if (target_opacity_ > opacity_) {
    opacity_ = (std::min)(opacity_ + step, target_opacity_);
  } else {
    opacity_ = (std::max)(opacity_ - step, target_opacity_);
  }

  sprite_->SetColor({0.0f, 0.0f, 0.0f, opacity_});

  bool reached_target = false;
  if (target_opacity_ > 0.5f && opacity_ >= 1.0f - FADE_EPSILON) {
    opacity_ = 1.0f;
    sprite_->SetColor({0.0f, 0.0f, 0.0f, 1.0f});
    reached_target = true;
  } else if (target_opacity_ < 0.5f && opacity_ <= FADE_EPSILON) {
    opacity_ = 0.0f;
    sprite_->SetColor({0.0f, 0.0f, 0.0f, 0.0f});
    overlay_go_->SetActive(false);
    reached_target = true;
  }

  if (reached_target && on_complete_) {
    auto callback = std::move(on_complete_);
    on_complete_ = nullptr;
    callback();
  }
}

void SceneTransitionOverlay::UpdateLayout(float screen_w, float screen_h) {
  sprite_->SetSize({screen_w, screen_h});
}

bool SceneTransitionOverlay::IsFading() const {
  return opacity_ != target_opacity_;
}

bool SceneTransitionOverlay::IsFadingIn() const {
  return target_opacity_ >= 1.0f;
}

bool SceneTransitionOverlay::IsFullyOpaque() const {
  return opacity_ >= 1.0f - FADE_EPSILON;
}
