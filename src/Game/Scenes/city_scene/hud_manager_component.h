#pragma once

#include <string>
#include <vector>

#include "Component/behavior_component.h"
#include "Graphic/Frame/frame_packet.h"

class UIGlassRenderer;
class UITextRenderer;
class UISpriteRenderer;
class GameObject;

class HudManagerComponent : public BehaviorComponent<HudManagerComponent> {
 public:
  struct Props {};

  HudManagerComponent(GameObject* owner, const Props& props = {});

  void OnInit() override;
  void OnUpdate(float dt) override;
  void OnRender(FramePacket& packet) override;

  void SetWave(int wave);
  void SetHealth(int hp);
  void SetGold(int gold);
  void ShowMessage(const std::wstring& text);
  void ShowAlert(const std::wstring& text);

 private:
  struct IconSlot {
    GameObject* root = nullptr;
    UIGlassRenderer* glass = nullptr;
    UISpriteRenderer* icon = nullptr;
  };

  void UpdateLayout();

  GameObject* info_panel_ = nullptr;
  UIGlassRenderer* info_glass_ = nullptr;
  UITextRenderer* wave_text_ = nullptr;
  UITextRenderer* hp_text_ = nullptr;
  UITextRenderer* gold_text_ = nullptr;

  GameObject* message_panel_ = nullptr;
  UIGlassRenderer* message_glass_ = nullptr;
  UITextRenderer* message_text_ = nullptr;
  float message_timer_ = 0.0f;
  bool message_visible_ = false;

  GameObject* alert_panel_ = nullptr;
  UIGlassRenderer* alert_glass_ = nullptr;
  UITextRenderer* alert_text_ = nullptr;
  float alert_timer_ = 0.0f;
  bool alert_visible_ = false;

  GameObject* hint_panel_ = nullptr;
  UIGlassRenderer* hint_glass_ = nullptr;
  UITextRenderer* hint_text_ = nullptr;

  std::vector<IconSlot> icon_slots_;

  int wave_ = 1;
  int hp_ = 100;
  int gold_ = 0;
};
