#pragma once

#include "SceneSetting/scene_transition_overlay.h"
#include "scene.h"

class InputSystem;
class TransformComponent;
class UIGlassRenderer;
class UISpriteRenderer;
class UITextRenderer;

class TitleScene : public IScene {
 public:
  void OnEnter(AssetManager& asset_manager) override;
  void OnExit() override;
  void OnPreUpdate(float dt) override;
  void OnRender(FramePacket& packet) override;

 private:
  void UpdateLayout();

  InputSystem* input_ = nullptr;
  TransformComponent* camera_transform_ = nullptr;

  UIGlassRenderer* logo_glass_ = nullptr;
  UISpriteRenderer* logo_sprite_ = nullptr;
  GameObject* logo_panel_ = nullptr;
  GameObject* logo_sprite_go_ = nullptr;

  UIGlassRenderer* start_glass_ = nullptr;
  GameObject* start_btn_ = nullptr;
  UITextRenderer* start_label_ = nullptr;
  GameObject* start_label_go_ = nullptr;

  UIGlassRenderer* leave_glass_ = nullptr;
  GameObject* leave_btn_ = nullptr;
  UITextRenderer* leave_label_ = nullptr;
  GameObject* leave_label_go_ = nullptr;

  struct Rect {
    float x, y, w, h;
  };
  Rect start_rect_{};
  Rect leave_rect_{};

  SceneTransitionOverlay transition_overlay_;
};
