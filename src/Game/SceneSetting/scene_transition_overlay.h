#pragma once

#include <functional>
#include <string>

class GameObject;
class IScene;
class UISpriteRenderer;

class SceneTransitionOverlay {
 public:
  void Create(IScene* scene, const std::string& name);

  void FadeIn(std::function<void()> on_complete = nullptr);
  void FadeOut(std::function<void()> on_complete = nullptr);

  void SetOpaque();
  void SetTransparent();

  void Update(float dt);
  void UpdateLayout(float screen_w, float screen_h);

  bool IsFading() const;
  bool IsFadingIn() const;
  bool IsFullyOpaque() const;

 private:
  static constexpr float FADE_SPEED = 1.0f;
  static constexpr float FADE_EPSILON = 0.005f;

  GameObject* overlay_go_ = nullptr;
  UISpriteRenderer* sprite_ = nullptr;

  float opacity_ = 0.0f;
  float target_opacity_ = 0.0f;
  std::function<void()> on_complete_;
};
