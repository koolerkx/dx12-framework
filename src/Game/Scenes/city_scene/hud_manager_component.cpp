#include "Scenes/city_scene/hud_manager_component.h"

#include <cstdio>

#include "Component/Renderer/ui_glass_renderer.h"
#include "Component/Renderer/ui_sprite_renderer.h"
#include "Component/Renderer/ui_text_renderer.h"
#include "Framework/Event/event_bus.hpp"
#include "Framework/Input/input.h"
#include "game_context.h"
#include "game_object.h"
#include "scene.h"
#include "Scenes/city_scene/city_scene_events.h"

namespace {

constexpr float DESIGN_HEIGHT = 1080.0f;
constexpr float UI_SCALE = 1.5f;
constexpr float SAFE_AREA = 48.0f;
constexpr float PADDING = 16.0f;
constexpr float TEXT_SIZE = 32.0f;
constexpr float SMALL_TEXT_SIZE = 28.0f;
constexpr float TEXT_LINE_HEIGHT = 36.0f;
constexpr float FADE_SPEED = 5.0f;
constexpr float FADE_EPSILON = 0.001f;

struct PanelLayout {
  float width;
  float height;
};

constexpr PanelLayout INFO_PANEL = {240.0f, 156.0f};
constexpr PanelLayout MESSAGE_PANEL = {480.0f, 68.0f};
constexpr PanelLayout ALERT_PANEL = {360.0f, 52.0f};
constexpr PanelLayout HINT_PANEL = {264.0f, 240.0f};
constexpr PanelLayout ICON_SLOT = {128.0f, 128.0f};
constexpr PanelLayout CONFIRM_BUTTON = {300.0f, 66.0f};
constexpr float BUTTON_GAP = 12.0f;

float MoveToward(float current, float target, float max_delta) {
  if (target > current) {
    return (std::min)(current + max_delta, target);
  }
  return (std::max)(current - max_delta, target);
}

}  // namespace

HudManagerComponent::HudManagerComponent(GameObject* owner, const Props& /*props*/) : BehaviorComponent(owner) {
}

void HudManagerComponent::OnInit() {
  auto* scene = static_cast<IScene*>(GetOwner()->GetScene());
  auto* hud_root = GetOwner();

  info_panel_ = scene->CreateGameObject("HUD_Info");
  info_panel_->SetParent(hud_root);
  info_glass_ = info_panel_->AddComponent<UIGlassRenderer>(UIGlassRenderer::Props{
    .size = {INFO_PANEL.width, INFO_PANEL.height},
    .layer_id = 1,
  });

  auto* wave_go = scene->CreateGameObject("HUD_WaveText");
  wave_go->SetParent(info_panel_);
  wave_text_ = wave_go->AddComponent<UITextRenderer>(UITextRenderer::Props{
    .text = L"Wave: 1",
    .pixel_size = TEXT_SIZE,
    .pivot = {0.0f, 0.0f},
  });

  auto* hp_go = scene->CreateGameObject("HUD_HPText");
  hp_go->SetParent(info_panel_);
  hp_text_ = hp_go->AddComponent<UITextRenderer>(UITextRenderer::Props{
    .text = L"HP: 100",
    .pixel_size = TEXT_SIZE,
    .pivot = {0.0f, 0.0f},
  });

  auto* gold_go = scene->CreateGameObject("HUD_GoldText");
  gold_go->SetParent(info_panel_);
  gold_text_ = gold_go->AddComponent<UITextRenderer>(UITextRenderer::Props{
    .text = L"Gold: 0",
    .pixel_size = TEXT_SIZE,
    .pivot = {0.0f, 0.0f},
  });

  auto* message_panel = scene->CreateGameObject("HUD_Message");
  message_panel->SetParent(hud_root);
  auto* message_glass = message_panel->AddComponent<UIGlassRenderer>(UIGlassRenderer::Props{
    .size = {MESSAGE_PANEL.width, MESSAGE_PANEL.height},
    .layer_id = 1,
  });

  auto* msg_text_go = scene->CreateGameObject("HUD_MessageText");
  msg_text_go->SetParent(message_panel);
  auto* message_text = msg_text_go->AddComponent<UITextRenderer>(UITextRenderer::Props{
    .text = L"",
    .pixel_size = TEXT_SIZE,
    .h_align = Text::HorizontalAlign::Center,
    .pivot = {0.5f, 0.0f},
  });

  message_fade_ = {
    .panel = message_panel,
    .glass = message_glass,
    .text = message_text,
    .base_tint_alpha = 0.15f,
  };
  message_panel->SetActive(false);

  auto* alert_panel = scene->CreateGameObject("HUD_Alert");
  alert_panel->SetParent(hud_root);
  auto* alert_glass = alert_panel->AddComponent<UIGlassRenderer>(UIGlassRenderer::Props{
    .size = {ALERT_PANEL.width, ALERT_PANEL.height},
    .tint_color = {1.0f, 0.0f, 0.0f, 0.3f},
    .layer_id = 1,
  });

  auto* alert_text_go = scene->CreateGameObject("HUD_AlertText");
  alert_text_go->SetParent(alert_panel);
  auto* alert_text = alert_text_go->AddComponent<UITextRenderer>(UITextRenderer::Props{
    .text = L"",
    .pixel_size = SMALL_TEXT_SIZE,
    .h_align = Text::HorizontalAlign::Center,
    .pivot = {0.5f, 0.0f},
  });

  alert_fade_ = {
    .panel = alert_panel,
    .glass = alert_glass,
    .text = alert_text,
    .base_tint_alpha = 0.15f,
  };
  alert_panel->SetActive(false);

  hint_panel_ = scene->CreateGameObject("HUD_Hint");
  hint_panel_->SetParent(hud_root);
  hint_glass_ = hint_panel_->AddComponent<UIGlassRenderer>(UIGlassRenderer::Props{
    .size = {HINT_PANEL.width, HINT_PANEL.height},
    .layer_id = 1,
  });

  auto* hint_text_go = scene->CreateGameObject("HUD_HintText");
  hint_text_go->SetParent(hint_panel_);
  hint_text_ = hint_text_go->AddComponent<UITextRenderer>(UITextRenderer::Props{
    .text = L"WASD: Move\nE: Interact\nSpace: Jump",
    .pixel_size = SMALL_TEXT_SIZE,
    .pivot = {0.0f, 0.0f},
  });

  auto* icon_go = scene->CreateGameObject("HUD_IconSlot_0");
  icon_go->SetParent(hud_root);
  auto* icon_glass = icon_go->AddComponent<UIGlassRenderer>(UIGlassRenderer::Props{
    .size = {ICON_SLOT.width, ICON_SLOT.height},
    .layer_id = 1,
  });

  auto* icon_sprite_go = scene->CreateGameObject("HUD_IconSprite_0");
  icon_sprite_go->SetParent(icon_go);
  auto* icon_sprite = icon_sprite_go->AddComponent<UISpriteRenderer>(UISpriteRenderer::Props{
    .texture_path = "Content/textures/tower-square-build-d.png",
    .size = {ICON_SLOT.width, ICON_SLOT.height},
  });
  icon_sprite->SetUVScale({1.0f, -1.0f});

  icon_slots_.push_back({.root = icon_go, .glass = icon_glass, .icon = icon_sprite});

  confirm_panel_ = scene->CreateGameObject("HUD_ConfirmPanel");
  confirm_panel_->SetParent(hud_root);
  confirm_panel_glass_ = confirm_panel_->AddComponent<UIGlassRenderer>(UIGlassRenderer::Props{
    .size = {CONFIRM_BUTTON.width, CONFIRM_BUTTON.height},
    .layer_id = 1,
  });

  auto* cost_go = scene->CreateGameObject("HUD_CostText");
  cost_go->SetParent(confirm_panel_);
  cost_text_ = cost_go->AddComponent<UITextRenderer>(UITextRenderer::Props{
    .text = L"Cost: 0",
    .pixel_size = TEXT_SIZE,
    .h_align = Text::HorizontalAlign::Center,
    .pivot = {0.5f, 0.0f},
  });

  auto create_button = [&](const char* name, const wchar_t* label_text) -> ButtonSlot {
    ButtonSlot slot;
    slot.root = scene->CreateGameObject(name);
    slot.root->SetParent(hud_root);
    slot.glass = slot.root->AddComponent<UIGlassRenderer>(UIGlassRenderer::Props{
      .size = {CONFIRM_BUTTON.width, CONFIRM_BUTTON.height},
      .layer_id = 2,
    });
    auto* label_go = scene->CreateGameObject(std::string(name) + "_Label");
    label_go->SetParent(slot.root);
    slot.label = label_go->AddComponent<UITextRenderer>(UITextRenderer::Props{
      .text = label_text,
      .pixel_size = SMALL_TEXT_SIZE,
      .h_align = Text::HorizontalAlign::Center,
      .pivot = {0.5f, 0.0f},
    });
    return slot;
  };

  confirm_button_ = create_button("HUD_ConfirmBtn", L"Confirm");
  cancel_button_ = create_button("HUD_CancelBtn", L"Cancel");
  SetConfirmPanelVisible(false);

  input_ = GetContext()->GetInput();
  SubscribeEvents();
  UpdateLayout();
}

void HudManagerComponent::SubscribeEvents() {
  auto& bus = *GetContext()->GetEventBus();

  event_scope_.Subscribe<GoldChangedEvent>(bus, [this](const GoldChangedEvent& e) {
    SetGold(e.gold);
  });

  event_scope_.Subscribe<HealthChangedEvent>(bus, [this](const HealthChangedEvent& e) {
    SetHealth(e.health);
  });

  event_scope_.Subscribe<WaveStartEvent>(bus, [this](const WaveStartEvent& e) {
    SetWave(e.wave);
    ShowMessage(L"Wave " + std::to_wstring(e.wave) + L"!", 3.0f);
    message_fade_.is_countdown = false;
  });

  event_scope_.Subscribe<WaveCountdownEvent>(bus, [this](const WaveCountdownEvent& e) {
    wchar_t buf[64];
    swprintf_s(buf, L"Next Wave in %.1fs", e.seconds_remaining);
    ShowCountdownMessage(buf);
  });

  event_scope_.Subscribe<InsufficientGoldEvent>(bus, [this](const InsufficientGoldEvent&) {
    ShowAlert(L"Not enough gold!", 3.0f);
  });

  event_scope_.Subscribe<EnemyReachedBaseEvent>(bus, [this](const EnemyReachedBaseEvent&) {
    ShowAlert(L"Base under attack!", 3.0f);
  });

  event_scope_.Subscribe<OverlapEnemyEvent>(bus, [this](const OverlapEnemyEvent&) {
    ShowAlert(L"Overlapping enemy!", 3.0f);
  });

  event_scope_.Subscribe<OverlapEnemySpawnEvent>(bus, [this](const OverlapEnemySpawnEvent&) {
    ShowAlert(L"Overlapping enemy spawn!", 3.0f);
  });

  event_scope_.Subscribe<TowerPlacementExitedEvent>(bus, [this](const TowerPlacementExitedEvent&) {
    icon_state_ = IconState::Normal;
    if (!icon_slots_.empty()) {
      icon_slots_[0].glass->SetTintColor({1.0f, 1.0f, 1.0f, 0.1f});
    }
    SetConfirmPanelVisible(false);
  });

  event_scope_.Subscribe<TowerPlacementSelectedEvent>(bus, [this](const TowerPlacementSelectedEvent& e) {
    cost_text_->SetText(L"Cost: " + std::to_wstring(e.cost));
    SetConfirmPanelVisible(true);
  });

  event_scope_.Subscribe<TowerPlacementCancelledEvent>(bus, [this](const TowerPlacementCancelledEvent&) {
    SetConfirmPanelVisible(false);
  });
}

void HudManagerComponent::OnUpdate(float dt) {
  UpdateFadePanel(message_fade_, dt);
  UpdateFadePanel(alert_fade_, dt);
  UpdateIconInteraction();
  UpdateConfirmPanelInteraction();
}

void HudManagerComponent::OnRender(FramePacket& /*packet*/) {
  UpdateLayout();
}

void HudManagerComponent::UpdateFadePanel(FadePanel& fade, float dt) {
  if (fade.auto_hide_delay >= 0.0f) {
    fade.auto_hide_delay -= dt;
    if (fade.auto_hide_delay <= 0.0f) {
      fade.target_opacity = 0.0f;
      fade.auto_hide_delay = -1.0f;
    }
  }

  if (fade.opacity == fade.target_opacity) return;

  fade.opacity = MoveToward(fade.opacity, fade.target_opacity, FADE_SPEED * dt);

  if (fade.opacity < FADE_EPSILON) {
    fade.opacity = 0.0f;
    fade.panel->SetActive(false);
    return;
  }

  fade.text->SetColor({1.0f, 1.0f, 1.0f, fade.opacity});
  fade.glass->SetTintAlpha(fade.base_tint_alpha * fade.opacity);
}

void HudManagerComponent::UpdateLayout() {
  auto* graphic = GetOwner()->GetContext()->GetGraphic();
  float screen_w = static_cast<float>(graphic->GetFrameBufferWidth());
  float screen_h = static_cast<float>(graphic->GetFrameBufferHeight());
  float s = (screen_h / DESIGN_HEIGHT) * UI_SCALE;

  auto set_pos = [](GameObject* go, float x, float y) {
    go->GetTransform()->SetPosition({x, y, 0.0f});
  };

  set_pos(info_panel_, SAFE_AREA * s, SAFE_AREA * s);
  info_glass_->SetSize({INFO_PANEL.width * s, INFO_PANEL.height * s});

  set_pos(wave_text_->GetOwner(), PADDING * s, PADDING * s);
  wave_text_->SetPixelSize(TEXT_SIZE * s);

  set_pos(hp_text_->GetOwner(), PADDING * s, (PADDING + TEXT_LINE_HEIGHT) * s);
  hp_text_->SetPixelSize(TEXT_SIZE * s);

  set_pos(gold_text_->GetOwner(), PADDING * s, (PADDING + TEXT_LINE_HEIGHT * 2.0f) * s);
  gold_text_->SetPixelSize(TEXT_SIZE * s);

  float msg_x = (screen_w - MESSAGE_PANEL.width * s) / 2.0f;
  set_pos(message_fade_.panel, msg_x, SAFE_AREA * s);
  message_fade_.glass->SetSize({MESSAGE_PANEL.width * s, MESSAGE_PANEL.height * s});

  set_pos(message_fade_.text->GetOwner(), MESSAGE_PANEL.width * s / 2.0f, PADDING * s);
  message_fade_.text->SetPixelSize(TEXT_SIZE * s);

  float alert_y = (SAFE_AREA + MESSAGE_PANEL.height + 12.0f) * s;
  float alert_x = (screen_w - ALERT_PANEL.width * s) / 2.0f;
  set_pos(alert_fade_.panel, alert_x, alert_y);
  alert_fade_.glass->SetSize({ALERT_PANEL.width * s, ALERT_PANEL.height * s});

  set_pos(alert_fade_.text->GetOwner(), ALERT_PANEL.width * s / 2.0f, PADDING * s);
  alert_fade_.text->SetPixelSize(SMALL_TEXT_SIZE * s);

  float hint_x = screen_w - (SAFE_AREA + HINT_PANEL.width) * s;
  set_pos(hint_panel_, hint_x, SAFE_AREA * s);
  hint_glass_->SetSize({HINT_PANEL.width * s, HINT_PANEL.height * s});

  set_pos(hint_text_->GetOwner(), PADDING * s, PADDING * s);
  hint_text_->SetPixelSize(SMALL_TEXT_SIZE * s);

  panel_rects_.clear();
  panel_rects_.push_back({SAFE_AREA * s, SAFE_AREA * s, INFO_PANEL.width * s, INFO_PANEL.height * s});
  panel_rects_.push_back({hint_x, SAFE_AREA * s, HINT_PANEL.width * s, HINT_PANEL.height * s});

  if (message_fade_.panel->IsActive()) {
    panel_rects_.push_back({msg_x, SAFE_AREA * s, MESSAGE_PANEL.width * s, MESSAGE_PANEL.height * s});
  }
  if (alert_fade_.panel->IsActive()) {
    panel_rects_.push_back({alert_x, alert_y, ALERT_PANEL.width * s, ALERT_PANEL.height * s});
  }

  for (size_t i = 0; i < icon_slots_.size(); ++i) {
    float icon_x = (SAFE_AREA + static_cast<float>(i) * 140.0f) * s;
    float icon_y = screen_h - (SAFE_AREA + ICON_SLOT.height) * s;
    float icon_size = ICON_SLOT.width * s;

    set_pos(icon_slots_[i].root, icon_x, icon_y);
    icon_slots_[i].glass->SetSize({icon_size, icon_size});
    icon_slots_[i].icon->SetSize({icon_size, icon_size});

    panel_rects_.push_back({icon_x, icon_y, icon_size, icon_size});
  }

  if (confirm_panel_->IsActive()) {
    float btn_w = CONFIRM_BUTTON.width * s;
    float btn_h = CONFIRM_BUTTON.height * s;
    float gap = BUTTON_GAP * s;
    float total_h = btn_h * 3.0f + gap * 2.0f;

    float base_x = screen_w - (SAFE_AREA * s + btn_w);
    float base_y = screen_h - (SAFE_AREA * s + total_h);

    set_pos(confirm_panel_, base_x, base_y);
    confirm_panel_glass_->SetSize({btn_w, btn_h});
    set_pos(cost_text_->GetOwner(), btn_w * 0.5f, (btn_h - TEXT_SIZE * s) * 0.5f);
    cost_text_->SetPixelSize(TEXT_SIZE * s);

    float confirm_y = base_y + btn_h + gap;
    set_pos(confirm_button_.root, base_x, confirm_y);
    confirm_button_.glass->SetSize({btn_w, btn_h});
    set_pos(confirm_button_.label->GetOwner(), btn_w * 0.5f, (btn_h - SMALL_TEXT_SIZE * s) * 0.5f);
    confirm_button_.label->SetPixelSize(SMALL_TEXT_SIZE * s);
    confirm_button_.rect = {base_x, confirm_y, btn_w, btn_h};

    float cancel_y = confirm_y + btn_h + gap;
    set_pos(cancel_button_.root, base_x, cancel_y);
    cancel_button_.glass->SetSize({btn_w, btn_h});
    set_pos(cancel_button_.label->GetOwner(), btn_w * 0.5f, (btn_h - SMALL_TEXT_SIZE * s) * 0.5f);
    cancel_button_.label->SetPixelSize(SMALL_TEXT_SIZE * s);
    cancel_button_.rect = {base_x, cancel_y, btn_w, btn_h};

    panel_rects_.push_back({base_x, base_y, btn_w, total_h});
  }
}

void HudManagerComponent::SetWave(int wave) {
  wave_ = wave;
  wave_text_->SetText(L"Wave: " + std::to_wstring(wave));
}

void HudManagerComponent::SetHealth(int hp) {
  hp_ = hp;
  hp_text_->SetText(L"HP: " + std::to_wstring(hp));
}

void HudManagerComponent::SetGold(int gold) {
  gold_ = gold;
  gold_text_->SetText(L"Gold: " + std::to_wstring(gold));
}

void HudManagerComponent::ShowMessage(const std::wstring& text, float duration) {
  message_fade_.text->SetText(text);
  message_fade_.target_opacity = 1.0f;
  message_fade_.auto_hide_delay = duration;
  message_fade_.is_countdown = false;
  if (!message_fade_.panel->IsActive()) {
    message_fade_.panel->SetActive(true);
    message_fade_.opacity = 0.0f;
    message_fade_.text->SetColor({1.0f, 1.0f, 1.0f, 0.0f});
    message_fade_.glass->SetTintAlpha(0.0f);
  }
}

void HudManagerComponent::ShowAlert(const std::wstring& text, float duration) {
  alert_fade_.text->SetText(text);
  alert_fade_.target_opacity = 1.0f;
  alert_fade_.auto_hide_delay = duration;
  alert_fade_.is_countdown = false;
  if (!alert_fade_.panel->IsActive()) {
    alert_fade_.panel->SetActive(true);
    alert_fade_.opacity = 0.0f;
    alert_fade_.text->SetColor({1.0f, 1.0f, 1.0f, 0.0f});
    alert_fade_.glass->SetTintAlpha(0.0f);
  }
}

void HudManagerComponent::ShowCountdownMessage(const std::wstring& text) {
  message_fade_.text->SetText(text);
  message_fade_.target_opacity = 1.0f;
  message_fade_.auto_hide_delay = -1.0f;
  message_fade_.is_countdown = true;
  if (!message_fade_.panel->IsActive()) {
    message_fade_.panel->SetActive(true);
    message_fade_.opacity = 0.0f;
    message_fade_.text->SetColor({1.0f, 1.0f, 1.0f, 0.0f});
    message_fade_.glass->SetTintAlpha(0.0f);
  }
}

bool HudManagerComponent::IsMouseOverUI(float mx, float my) const {
  for (const auto& r : panel_rects_) {
    if (mx >= r.x && mx <= r.x + r.w && my >= r.y && my <= r.y + r.h) {
      return true;
    }
  }
  return false;
}

int HudManagerComponent::HitTestIconSlot(float mx, float my) const {
  auto* graphic = GetOwner()->GetContext()->GetGraphic();
  float screen_h = static_cast<float>(graphic->GetFrameBufferHeight());
  float s = (screen_h / DESIGN_HEIGHT) * UI_SCALE;

  for (size_t i = 0; i < icon_slots_.size(); ++i) {
    float icon_x = (SAFE_AREA + static_cast<float>(i) * 140.0f) * s;
    float icon_y = screen_h - (SAFE_AREA + ICON_SLOT.height) * s;
    float icon_size = ICON_SLOT.width * s;

    if (mx >= icon_x && mx <= icon_x + icon_size && my >= icon_y && my <= icon_y + icon_size) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

void HudManagerComponent::UpdateIconInteraction() {
  auto [mx, my] = input_->GetMousePosition();
  int hovered_slot = HitTestIconSlot(mx, my);

  if (input_->GetMouseButtonDown(Mouse::Button::Left) && hovered_slot == 0) {
    if (icon_state_ == IconState::Active) {
      icon_state_ = IconState::Normal;
    } else {
      icon_state_ = IconState::Active;
    }
    GetContext()->GetEventBus()->Emit(ToggleTowerPlacementEvent{});
  } else if (icon_state_ != IconState::Active) {
    icon_state_ = (hovered_slot == 0) ? IconState::Hovered : IconState::Normal;
  }

  if (icon_slots_.empty()) return;

  Math::Vector4 tint;
  switch (icon_state_) {
    case IconState::Hovered:
      tint = {0.3f, 0.5f, 1.0f, 0.2f};
      break;
    case IconState::Active:
      tint = {1.0f, 0.85f, 0.0f, 0.25f};
      break;
    default:
      tint = {1.0f, 1.0f, 1.0f, 0.1f};
      break;
  }
  icon_slots_[0].glass->SetTintColor(tint);
}

void HudManagerComponent::SetConfirmPanelVisible(bool visible) {
  confirm_panel_->SetActive(visible);
  confirm_button_.root->SetActive(visible);
  cancel_button_.root->SetActive(visible);
}

void HudManagerComponent::UpdateConfirmPanelInteraction() {
  if (!confirm_panel_->IsActive()) return;

  auto [mx, my] = input_->GetMousePosition();

  auto hit_test = [](float mx, float my, const PanelRect& r) {
    return mx >= r.x && mx <= r.x + r.w && my >= r.y && my <= r.y + r.h;
  };

  static const Math::Vector4 HOVER_TINT = {0.3f, 0.5f, 1.0f, 0.2f};
  static const Math::Vector4 DEFAULT_TINT = {1.0f, 1.0f, 1.0f, 0.1f};

  bool over_confirm = hit_test(mx, my, confirm_button_.rect);
  bool over_cancel = hit_test(mx, my, cancel_button_.rect);

  confirm_button_.glass->SetTintColor(over_confirm ? HOVER_TINT : DEFAULT_TINT);
  cancel_button_.glass->SetTintColor(over_cancel ? HOVER_TINT : DEFAULT_TINT);

  if (input_->GetMouseButtonDown(Mouse::Button::Left)) {
    if (over_confirm) {
      GetContext()->GetEventBus()->Emit(TowerPlacementConfirmedEvent{});
    } else if (over_cancel) {
      GetContext()->GetEventBus()->Emit(TowerPlacementCancelledEvent{});
    }
  }
}
