#include "Scenes/city_scene/hud_manager_component.h"

#include <cstdio>

#include "Component/Renderer/ui_glass_renderer.h"
#include "Component/Renderer/ui_sprite_renderer.h"
#include "Component/Renderer/ui_text_renderer.h"
#include "game_context.h"
#include "game_object.h"
#include "scene.h"
#include "Scenes/city_scene/city_scene_events.h"

namespace {

constexpr float DESIGN_HEIGHT = 1080.0f;
constexpr float SAFE_AREA = 48.0f;
constexpr float PADDING = 16.0f;
constexpr float TEXT_SIZE = 28.0f;
constexpr float SMALL_TEXT_SIZE = 24.0f;
constexpr float TEXT_LINE_HEIGHT = 38.0f;
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
}

void HudManagerComponent::OnUpdate(float dt) {
  UpdateFadePanel(message_fade_, dt);
  UpdateFadePanel(alert_fade_, dt);
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
  float s = screen_h / DESIGN_HEIGHT;

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

  for (size_t i = 0; i < icon_slots_.size(); ++i) {
    float icon_x = (SAFE_AREA + static_cast<float>(i) * 140.0f) * s;
    float icon_y = screen_h - (SAFE_AREA + ICON_SLOT.height) * s;
    float icon_size = ICON_SLOT.width * s;

    set_pos(icon_slots_[i].root, icon_x, icon_y);
    icon_slots_[i].glass->SetSize({icon_size, icon_size});
    icon_slots_[i].icon->SetSize({icon_size, icon_size});
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
