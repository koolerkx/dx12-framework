#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Component/behavior_component.h"
#include "Framework/Event/event_scope.hpp"
#include "Graphic/Frame/frame_packet.h"

class UIGlassRenderer;
class UITextRenderer;
class UISpriteRenderer;
class InputSystem;
class GameObject;

class HudManagerComponent : public BehaviorComponent<HudManagerComponent> {
 public:
  struct Props {};

  HudManagerComponent(GameObject* owner, const Props& props = {});

  void OnInit() override;
  void OnUpdate(float dt) override;
  void OnRender(FramePacket& packet) override;

  bool IsMouseOverUI(float mx, float my) const;
  int HitTestIconSlot(float mx, float my) const;

 private:
  struct PanelRect {
    float x, y, w, h;
  };

  enum class IconState : uint8_t { Normal, Hovered, Active };
  struct IconSlot {
    GameObject* root = nullptr;
    UIGlassRenderer* glass = nullptr;
    UISpriteRenderer* icon = nullptr;
  };

  struct FadePanel {
    GameObject* panel = nullptr;
    UIGlassRenderer* glass = nullptr;
    UITextRenderer* text = nullptr;
    float opacity = 0.0f;
    float target_opacity = 0.0f;
    float auto_hide_delay = -1.0f;
    float base_tint_alpha = 0.15f;
    bool is_countdown = false;
  };

  void SetWave(int wave);
  void SetHealth(int hp);
  void SetGold(int gold);
  void ShowMessage(const std::wstring& text, float duration = 3.0f);
  void ShowAlert(const std::wstring& text, float duration = 3.0f);
  void ShowCountdownMessage(const std::wstring& text);

  void UpdateLayout();
  void UpdateFadePanel(FadePanel& fade, float dt);
  void UpdateIconInteraction();
  void UpdateConfirmPanelInteraction();
  void SetConfirmPanelVisible(bool visible);
  void SubscribeEvents();

  EventScope event_scope_;

  GameObject* info_panel_ = nullptr;
  UIGlassRenderer* info_glass_ = nullptr;
  UITextRenderer* wave_text_ = nullptr;
  UITextRenderer* hp_text_ = nullptr;
  UITextRenderer* gold_text_ = nullptr;

  FadePanel message_fade_;
  FadePanel alert_fade_;

  GameObject* hint_panel_ = nullptr;
  UIGlassRenderer* hint_glass_ = nullptr;
  UITextRenderer* hint_text_ = nullptr;

  struct ButtonSlot {
    GameObject* root = nullptr;
    UIGlassRenderer* glass = nullptr;
    UITextRenderer* label = nullptr;
    PanelRect rect = {};
  };

  std::vector<IconSlot> icon_slots_;
  std::vector<PanelRect> panel_rects_;

  GameObject* confirm_panel_ = nullptr;
  UIGlassRenderer* confirm_panel_glass_ = nullptr;
  UITextRenderer* cost_text_ = nullptr;
  ButtonSlot confirm_button_;
  ButtonSlot cancel_button_;

  IconState icon_state_ = IconState::Normal;
  InputSystem* input_ = nullptr;

  int wave_ = 1;
  int hp_ = 100;
  int gold_ = 0;
};
